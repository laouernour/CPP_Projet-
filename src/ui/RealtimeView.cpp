#include "ui/RealtimeView.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <list>
#include <string>

#include "bank/Bank.h"
#include "bank/Cashier.h"
#include "bank/Queue.h"
#include "client/AbstractClient.h"
#include "client/AbstractOperation.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationEntry.h"
#include "simulation/StatisticManager.h"

namespace
{
const wchar_t* const CLASS_NAME = L"Bank2RealtimeView";
const int TIMER_ID = 1;

// Couleurs
const COLORREF COL_BG        = RGB(245, 246, 248);
const COLORREF COL_PANEL     = RGB(255, 255, 255);
const COLORREF COL_BORDER    = RGB(200, 204, 210);
const COLORREF COL_FREE      = RGB(198, 239, 206);   // caissier libre (vert)
const COLORREF COL_BUSY      = RGB(255, 199, 206);   // caissier occupé (rouge)
const COLORREF COL_CLIENT    = RGB(197, 224, 255);   // client normal (bleu)
const COLORREF COL_VIP       = RGB(255, 214, 130);   // client VIP (or)
const COLORREF COL_TEXT      = RGB(40, 44, 52);

// Libellé court d'une opération.
const char* operationLabel(const AbstractClient* client)
{
    const std::string type = client->getOperation()->typeName();
    if (type == "consultation") return "Consultation";
    if (type == "transfer")     return "Virement";
    return "Retrait";
}

// Dessine un rectangle rempli avec bordure.
void fillBox(HDC hdc, RECT r, COLORREF fill)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN   pen   = CreatePen(PS_SOLID, 1, COL_BORDER);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen   = SelectObject(hdc, pen);
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void drawText(HDC hdc, int x, int y, const std::string& text)
{
    TextOutA(hdc, x, y, text.c_str(), static_cast<int>(text.size()));
}

void drawTextCentered(HDC hdc, RECT r, const std::string& text)
{
    RECT copy = r;
    DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &copy,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
}   // namespace


RealtimeView::RealtimeView(Simulation& simulation, int msPerTick)
    : m_simulation(simulation), m_msPerTick(msPerTick)
{
    m_simulation.setVerbose(false);   // Pas de trace console pendant l'animation
}


// Déclaration de la procédure de fenêtre (définie plus bas).
static LRESULT CALLBACK bank2WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


void RealtimeView::run()
{
    HINSTANCE instance = GetModuleHandle(nullptr);

    WNDCLASSW wc = {};
    wc.lpfnWndProc   = bank2WndProc;
    wc.hInstance     = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Simulation d'agence bancaire",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 940, 620,
        nullptr, nullptr, instance, this);   // 'this' -> lpCreateParams

    if (hwnd == nullptr)
    {
        // Pas d'interface graphique disponible : on déroule quand même la simulation
        while (m_simulation.step())
        {
        }
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


// --- Dessin ---------------------------------------------------------------

static void paint(HWND hwnd, Simulation& sim)
{
    RECT client;
    GetClientRect(hwnd, &client);

    PAINTSTRUCT ps;
    HDC windowDc = BeginPaint(hwnd, &ps);

    // Double buffering pour éviter le scintillement
    HDC hdc = CreateCompatibleDC(windowDc);
    HBITMAP bmp = CreateCompatibleBitmap(windowDc, client.right, client.bottom);
    HGDIOBJ oldBmp = SelectObject(hdc, bmp);

    HBRUSH bg = CreateSolidBrush(COL_BG);
    FillRect(hdc, &client, bg);
    DeleteObject(bg);

    HFONT titleFont = CreateFontA(-20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    HFONT normalFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    HFONT smallFont = CreateFontA(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COL_TEXT);

    const SimulationEntry& entry = sim.getEntry();
    const StatisticManager& stats = sim.getStatistics();
    const Bank& bank = sim.getBank();

    // Titre + horloge
    HGDIOBJ oldFont = SelectObject(hdc, titleFont);
    drawText(hdc, 20, 14, "Agence bancaire - simulation en temps reel");
    SelectObject(hdc, normalFont);
    const int shownTime = sim.isFinished() ? entry.getSimulationDuration() : sim.getCurrentTime();
    drawText(hdc, 20, 44,
        "Temps : " + std::to_string(shownTime) + " / " + std::to_string(entry.getSimulationDuration()));

    // --- Caissiers ---
    drawText(hdc, 20, 78, "Caissiers");
    const auto& cashiers = bank.getCashiers();
    int cx = 20;
    const int cw = 210, ch = 90, cgap = 16;
    int index = 1;
    for (const Cashier* cashier : cashiers)
    {
        RECT box = { cx, 98, cx + cw, 98 + ch };
        fillBox(hdc, box, cashier->isFree() ? COL_FREE : COL_BUSY);

        SelectObject(hdc, normalFont);
        drawText(hdc, cx + 12, 106, "Caissier " + std::to_string(index));

        SelectObject(hdc, smallFont);
        if (cashier->isFree())
        {
            drawText(hdc, cx + 12, 130, "libre");
        }
        else
        {
            const AbstractClient* served = cashier->getServingClient();
            drawText(hdc, cx + 12, 130,
                std::string(operationLabel(served)) +
                (served->isPriority() ? " (VIP)" : ""));
            drawText(hdc, cx + 12, 150,
                "reste : " + std::to_string(cashier->getRemainingServiceTime()));
        }

        cx += cw + cgap;
        ++index;
    }

    // --- File d'attente ---
    const Queue& queue = bank.getQueue();
    SelectObject(hdc, normalFont);
    drawText(hdc, 20, 210, "File d'attente (" + std::to_string(queue.size()) + ")");

    int qx = 20;
    const int qy = 232, qw = 96, qh = 74, qgap = 8;
    const int maxBoxes = 9;
    int drawn = 0;
    for (const AbstractClient* client : queue.items())
    {
        if (drawn >= maxBoxes)
        {
            SelectObject(hdc, smallFont);
            drawText(hdc, qx, qy + 24, "+" + std::to_string(queue.size() - maxBoxes) + " ...");
            break;
        }
        RECT box = { qx, qy, qx + qw, qy + qh };
        fillBox(hdc, box, client->isPriority() ? COL_VIP : COL_CLIENT);

        SelectObject(hdc, smallFont);
        drawText(hdc, qx + 8, qy + 6, client->isPriority() ? "VIP" : "Client");
        drawText(hdc, qx + 8, qy + 24, operationLabel(client));
        drawText(hdc, qx + 8, qy + 42, "pat. " + std::to_string(client->getPatienceTime()));

        qx += qw + qgap;
        ++drawn;
    }
    if (queue.size() == 0)
    {
        SelectObject(hdc, smallFont);
        drawText(hdc, 20, qy + 24, "(vide)");
    }

    // --- Statistiques en direct ---
    RECT panel = { 20, 340, 910, 560 };
    fillBox(hdc, panel, COL_PANEL);
    SelectObject(hdc, normalFont);
    drawText(hdc, 36, 352, "Statistiques");
    SelectObject(hdc, smallFont);
    int sy = 380;
    const int lh = 24;
    drawText(hdc, 36, sy, "Clients servis        : " + std::to_string(stats.servedClientCount())); sy += lh;
    drawText(hdc, 36, sy, "Clients non servis    : " + std::to_string(stats.nonServedClientCount())); sy += lh;

    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f", stats.calculateAverageClientWaitingTime());
    drawText(hdc, 36, sy, std::string("Temps d'attente moyen : ") + buf); sy += lh;
    snprintf(buf, sizeof(buf), "%.2f", stats.calculateAverageClientServiceTime());
    drawText(hdc, 36, sy, std::string("Temps de service moyen: ") + buf); sy += lh;
    snprintf(buf, sizeof(buf), "%.2f %%", stats.calculateAverageCashierOccupationRate(entry.getCashierCount()));
    drawText(hdc, 36, sy, std::string("Occupation caissiers  : ") + buf); sy += lh;
    snprintf(buf, sizeof(buf), "%.2f %%", stats.calculateClientSatisfactionRate());
    drawText(hdc, 36, sy, std::string("Satisfaction clients  : ") + buf); sy += lh;

    // --- Fin de simulation ---
    if (sim.isFinished())
    {
        RECT banner = { 20, 566, 910, 600 };
        fillBox(hdc, banner, COL_FREE);
        SelectObject(hdc, normalFont);
        drawTextCentered(hdc, banner, "SIMULATION TERMINEE - resultats enregistres dans la base de donnees");
    }

    SelectObject(hdc, oldFont);

    // Copie du buffer vers la fenêtre
    BitBlt(windowDc, 0, 0, client.right, client.bottom, hdc, 0, 0, SRCCOPY);

    DeleteObject(titleFont);
    DeleteObject(normalFont);
    DeleteObject(smallFont);
    SelectObject(hdc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(hdc);

    EndPaint(hwnd, &ps);
}


static LRESULT CALLBACK bank2WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE)
    {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        RealtimeView* view = static_cast<RealtimeView*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(view));
        SetTimer(hwnd, TIMER_ID, static_cast<UINT>(view->tickInterval()), nullptr);
        return 0;
    }

    RealtimeView* view =
        reinterpret_cast<RealtimeView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_TIMER:
        if (view != nullptr)
        {
            Simulation& sim = view->simulation();
            if (!sim.isFinished())
            {
                sim.step();                          // Avance d'une unité de temps
            }
            else
            {
                KillTimer(hwnd, TIMER_ID);            // Plus rien à animer
            }
            InvalidateRect(hwnd, nullptr, FALSE);     // Redessine
        }
        return 0;

    case WM_PAINT:
        if (view != nullptr)
        {
            paint(hwnd, view->simulation());
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
