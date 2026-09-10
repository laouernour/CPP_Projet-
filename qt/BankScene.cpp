#include "BankScene.h"

#include <algorithm>
#include <cmath>
#include <QFont>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
#include "bank/Bank.h"
#include "bank/Cashier.h"
#include "bank/Queue.h"
#include "client/AbstractClient.h"
#include "client/AbstractOperation.h"
#include "simulation/Simulation.h"
#include "simulation/StatisticManager.h"

namespace
{
constexpr double kWidth = 1120.0;
constexpr double kWalkSpeed = 285.0; // Pixels logiques par seconde, jamais par frame.
constexpr int kColumns = 8;
constexpr double kRowSpacing = 166.0;
constexpr double kQueueFeet = 290.0;
const QColor kInk("#203c46");
const QColor kMuted("#788b92");
const QColor kTeal("#167b72");
const QColor kBlue("#438cbc");
const QColor kGold("#d89b30");
const QColor kRed("#dc5657");
const QColor kOrange("#c56c31");
const QPointF kDoor(100.0, 200.0);

QString operationLabel(const AbstractClient* client)
{
    const std::string name = client->getOperation()->typeName();
    if (name == "consultation") return QStringLiteral("Consultation");
    if (name == "transfer") return QStringLiteral("Virement");
    return QStringLiteral("Retrait");
}

void label(QPainter& p, const QRectF& rect, const QString& text,
           int size, const QColor& color, bool bold = false,
           Qt::Alignment alignment = Qt::AlignCenter)
{
    QFont font(QStringLiteral("Segoe UI"));
    font.setPixelSize(size);
    font.setBold(bold);
    p.setFont(font);
    p.setPen(color);
    p.drawText(rect, alignment, text);
}

void person(QPainter& p, QPointF feet, QColor color, double phase, bool walking)
{
    const double swing = walking ? std::sin(phase) * 6.0 : 0.0;
    const double bob = walking ? -std::abs(std::sin(phase)) * 1.6 : 0.0;
    p.save();
    p.translate(feet);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(30, 55, 60, 20));
    p.drawEllipse(QRectF(-13, -3, 26, 7));
    p.translate(0, bob);
    p.setPen(QPen(color.darker(120), 4.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(0, -20), QPointF(-7 + swing, 0));
    p.drawLine(QPointF(0, -20), QPointF(7 - swing, 0));
    p.setPen(QPen(color, 5.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(0, -39), QPointF(0, -20));
    p.drawLine(QPointF(0, -36), QPointF(-11 - swing * 0.5, -23));
    p.drawLine(QPointF(0, -36), QPointF(11 + swing * 0.5, -23));
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawEllipse(QPointF(0, -50), 9, 9);
    p.setBrush(QColor(255, 255, 255, 200));
    p.drawEllipse(QPointF(-2.5, -51), 1.2, 1.2);
    p.drawEllipse(QPointF(2.5, -51), 1.2, 1.2);
    p.restore();
}

void star(QPainter& p, QPointF center)
{
    QPolygonF shape;
    for (int i = 0; i < 10; ++i)
    {
        const double angle = -1.57079632679 + i * 0.62831853071;
        const double radius = i % 2 == 0 ? 6.5 : 3.0;
        shape << center + QPointF(std::cos(angle) * radius, std::sin(angle) * radius);
    }
    p.setPen(Qt::NoPen);
    p.setBrush(kGold);
    p.drawPolygon(shape);
}
}

BankScene::BankScene(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("bankScene"));
    setMinimumSize(640, 280);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAccessibleName(tr("Agence bancaire animée"));
    m_animationTimer = new QTimer(this);
    m_animationTimer->setObjectName(QStringLiteral("animationTimer"));
    m_animationTimer->setTimerType(Qt::PreciseTimer);
    m_animationTimer->setInterval(16);
    connect(m_animationTimer, &QTimer::timeout, this, &BankScene::advanceAnimation);
    m_frameClock.start();
    m_animationTimer->start();
    updateCanvasSize();
}

BankScene::~BankScene() = default;

double BankScene::cashierTop() const { return 450.0 + (m_queueRows - 1) * kRowSpacing; }
double BankScene::canvasHeight() const { return cashierTop() + 220.0; }

double BankScene::sceneScale() const
{
    return std::min(width() / kWidth, height() / canvasHeight());
}

QPointF BankScene::queuePosition(int index) const
{
    // Une file serpentine : chaque index conserve une place distincte.
    const int row = index / kColumns;
    const int column = row % 2 == 0 ? index % kColumns : kColumns - 1 - index % kColumns;
    return QPointF(226.0 + column * 98.0, kQueueFeet + row * kRowSpacing);
}

QPointF BankScene::servicePosition(int index) const
{
    const double count = std::max<std::size_t>(1, m_cashiers.size());
    return QPointF(140.0 + (index + 0.5) * 840.0 / count, cashierTop() - 14.0);
}

void BankScene::updateCanvasSize()
{
    update(); // Toute la scène tient dans la surface disponible, même avec une longue file.
}

void BankScene::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCanvasSize();
}

BankScene::ClientVisual* BankScene::ensureVisual(const AbstractClient* client)
{
    const auto existing = m_index.find(client);
    if (existing != m_index.end()) return existing->second;
    if (m_seen.count(client)) return nullptr; // Un départ animé ne réapparaît jamais.

    auto visual = std::make_unique<ClientVisual>();
    visual->model = client;
    // Emplacements extérieurs distincts, même lors d'une synchronisation groupée.
    std::unordered_set<int> taken;
    for (const auto& other : m_visuals)
        if (other->state == VisualState::Entering) taken.insert(other->arrivalSlot);
    while (taken.count(visual->arrivalSlot)) ++visual->arrivalSlot;
    visual->position = QPointF(30.0, 200.0 + visual->arrivalSlot * 32.0);
    visual->shownPatience = visual->targetPatience = client->getPatienceTime();
    setRoute(*visual, {kDoor, QPointF(150, 200)});
    ClientVisual* raw = visual.get();
    m_seen.insert(client);
    m_index.emplace(client, raw);
    m_visuals.push_back(std::move(visual));
    return raw;
}

void BankScene::syncWith(const Simulation& simulation)
{
    const auto& bank = simulation.getBank();
    const auto& stats = simulation.getStatistics();
    m_maxPatience = std::max(1, simulation.getEntry().getClientPatienceTime());
    m_queueRows = std::max(m_queueRows, (bank.getQueue().size() + kColumns - 1) / kColumns);
    const auto& cashiers = bank.getCashiers();
    m_cashiers.resize(cashiers.size());

    int queueIndex = 0;
    for (const auto* client : bank.getQueue().items())
    {
        if (auto* visual = ensureVisual(client))
        {
            visual->modelState = ModelState::Waiting;
            visual->queueIndex = queueIndex;
            visual->cashierIndex = -1;
        }
        ++queueIndex;
    }
    for (std::size_t i = 0; i < cashiers.size(); ++i)
    {
        auto& snapshot = m_cashiers[i];
        snapshot.client = cashiers[i]->getServingClient();
        snapshot.remaining = cashiers[i]->getRemainingServiceTime();
        snapshot.total = snapshot.client ? snapshot.client->getOperation()->getServiceTime() : 1;
        if (snapshot.client)
        {
            if (auto* visual = ensureVisual(snapshot.client))
            {
                visual->modelState = ModelState::Serving;
                visual->cashierIndex = static_cast<int>(i);
            }
        }
    }
    // Listes du moteur uniquement : aucune déduction d'abandon à partir d'une couleur.
    const auto& served = stats.servedClients();
    while (m_servedSeen < served.size())
        if (auto* visual = ensureVisual(served[m_servedSeen++]))
            visual->modelState = ModelState::Served;
    const auto& abandoned = stats.nonServedClients();
    while (m_abandonedSeen < abandoned.size())
        if (auto* visual = ensureVisual(abandoned[m_abandonedSeen++]))
            visual->modelState = ModelState::Abandoned;

    for (const auto& visual : m_visuals)
    {
        visual->targetPatience = visual->model->getPatienceTime();
        if (visual->targetPatience == 0) visual->shownPatience = 0;
        // Une affectation immédiate au guichet ne court-circuite jamais la porte.
        if (visual->state != VisualState::Entering) setDestination(*visual);
    }
    updateCanvasSize();
    update();
}

void BankScene::setRoute(ClientVisual& visual, std::vector<QPointF> route)
{
    visual.route = std::move(route);
    visual.waypoint = 0;
}

void BankScene::setDestination(ClientVisual& visual)
{
    if (visual.modelState == ModelState::Served || visual.modelState == ModelState::Abandoned)
    {
        if (visual.state != VisualState::Leaving && visual.state != VisualState::Gone)
        {
            // Sortir par une allée, sans traverser les personnes immobiles de la file.
            const double aisleY = visual.modelState == ModelState::Served
                ? visual.position.y() - 38.0 : visual.position.y() + 36.0;
            visual.state = VisualState::Leaving;
            // Couloir à droite puis porte de sortie distincte de l'entrée.
            setRoute(visual, {QPointF(visual.position.x(), aisleY), QPointF(1000, aisleY),
                             QPointF(1000, cashierTop() + 165),
                             QPointF(1060, cashierTop() + 165),
                             QPointF(1150, cashierTop() + 165)});
        }
        return;
    }
    if (visual.modelState == ModelState::Serving)
    {
        const QPointF target = servicePosition(visual.cashierIndex);
        if (visual.state == VisualState::BeingServed && visual.position == target) return;
        if (visual.state == VisualState::GoingToCashier && !visual.route.empty()
            && visual.route.back() == target) return;
        visual.state = VisualState::GoingToCashier;
        setRoute(visual, {QPointF(visual.position.x(), cashierTop() - 85),
                         QPointF(target.x(), cashierTop() - 85), target});
        return;
    }
    const QPointF target = queuePosition(visual.queueIndex);
    if (visual.position == target)
    {
        visual.state = VisualState::Waiting;
        visual.moving = false;
        return;
    }
    if (visual.state == VisualState::JoiningQueue && !visual.route.empty()
        && visual.route.back() == target) return;
    const bool fromEntrance = visual.state == VisualState::Entering;
    visual.state = VisualState::JoiningQueue;
    if (fromEntrance)
        setRoute(visual, {QPointF(150, target.y() + 36), QPointF(target.x(), target.y() + 36), target});
    else
        setRoute(visual, {target});
}

void BankScene::finishRoute(ClientVisual& visual)
{
    visual.moving = false;
    switch (visual.state)
    {
    case VisualState::Entering:
        setDestination(visual);
        break;
    case VisualState::JoiningQueue:
        visual.state = VisualState::Waiting;
        break;
    case VisualState::GoingToCashier:
        visual.state = VisualState::BeingServed;
        break;
    case VisualState::Leaving:
        visual.state = VisualState::Gone;
        break;
    default: break;
    }
}

bool BankScene::hasPendingTransitions() const
{
    for (const auto& visual : m_visuals)
        if (visual->state == VisualState::Entering || visual->state == VisualState::JoiningQueue
            || visual->state == VisualState::GoingToCashier) return true;
    return false;
}

void BankScene::setPaused(bool paused)
{
    m_paused = paused;
    m_frameClock.restart();
}

void BankScene::advanceAnimation()
{
    const double seconds = m_frameClock.nsecsElapsed() / 1.0e9;
    m_frameClock.restart();
    // En cas de fenêtre déplacée ou de retard système, éviter un grand saut.
    if (!m_paused) advance(std::min(seconds, 0.05));
}

void BankScene::advance(double seconds)
{
    for (auto it = m_visuals.begin(); it != m_visuals.end();)
    {
        auto& visual = **it;
        double budget = kWalkSpeed * seconds;
        double travelled = 0;
        while (budget > 0 && visual.waypoint < visual.route.size())
        {
            const QPointF delta = visual.route[visual.waypoint] - visual.position;
            const double distance = std::hypot(delta.x(), delta.y());
            if (distance <= budget)
            {
                visual.position = visual.route[visual.waypoint++]; // Arrêt exact.
                budget -= distance;
                travelled += distance;
                if (visual.waypoint == visual.route.size()) finishRoute(visual);
            }
            else
            {
                visual.position += delta * (budget / distance);
                travelled += budget;
                budget = 0;
            }
        }
        visual.moving = travelled > 0 && visual.waypoint < visual.route.size();
        visual.walkPhase += travelled * 0.09;
        // Interpolation vers la dernière valeur connue, jamais de patience inventée.
        const double patienceStep = seconds * m_maxPatience / 0.25;
        visual.shownPatience = std::max(visual.targetPatience, visual.shownPatience - patienceStep);
        if (visual.state == VisualState::Gone)
        {
            m_index.erase(visual.model);
            it = m_visuals.erase(it);
        }
        else ++it;
    }
    update();
}

void BankScene::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#f3f6f5"));
    const double scale = sceneScale();
    p.translate((width() - kWidth * scale) / 2.0,
                (height() - canvasHeight() * scale) / 2.0);
    p.scale(scale, scale);
    drawAgency(p);
    for (std::size_t i = 0; i < m_cashiers.size(); ++i) drawCashier(p, static_cast<int>(i));
    // Ordre de profondeur stable pour les personnages qui se croisent.
    std::vector<const ClientVisual*> ordered;
    for (const auto& visual : m_visuals) ordered.push_back(visual.get());
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) {
        return a->position.y() < b->position.y();
    });
    for (const auto* visual : ordered) drawClient(p, *visual);
}

void BankScene::drawAgency(QPainter& p) const
{
    const QRectF room(100, 95, 960, cashierTop() + 90);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(30, 60, 55, 9));
    p.drawRoundedRect(room.translated(0, 5), 20, 20);
    p.setBrush(Qt::white);
    p.setPen(QPen(QColor("#dce6e2"), 1.5));
    p.drawRoundedRect(room, 20, 20);

    label(p, QRectF(102, 20, 300, 25), tr("L’AGENCE"), 17, kInk, true, Qt::AlignLeft | Qt::AlignVCenter);
    const QString subtitle = m_cashiers.size() == 3 ? tr("Une file commune · Trois guichets")
        : tr("Une file commune · %1 guichet(s)").arg(m_cashiers.size());
    label(p, QRectF(102, 49, 400, 22), subtitle, 12, kMuted,
          false, Qt::AlignLeft | Qt::AlignVCenter);
    p.setPen(Qt::NoPen);
    p.setBrush(kBlue); p.drawEllipse(QPointF(771, 41), 4, 4);
    label(p, QRectF(785, 30, 60, 22), tr("Client"), 12, kMuted, false, Qt::AlignLeft | Qt::AlignVCenter);
    star(p, QPointF(868, 41));
    label(p, QRectF(880, 30, 40, 22), tr("VIP"), 12, kMuted);
    p.setPen(Qt::NoPen); p.setBrush(kRed); p.drawEllipse(QPointF(944, 41), 4, 4);
    label(p, QRectF(955, 30, 108, 22), tr("Patience épuisée"), 11, kMuted);

    // Porte vitrée ouverte et chemin d'entrée.
    p.setPen(QPen(QColor("#eff8f5"), 9));
    p.drawLine(QPointF(100, 164), QPointF(100, 226));
    p.setPen(QPen(QColor("#77b5a6"), 3));
    p.drawLine(QPointF(100, 164), QPointF(130, 149));
    p.drawLine(QPointF(100, 226), QPointF(130, 241));
    label(p, QRectF(8, 117, 84, 25), tr("ENTRÉE"), 11, kTeal, true);
    label(p, QRectF(14, 143, 66, 22), QStringLiteral("→"), 25, kTeal);

    const QRectF queueArea(174, 141, 812, m_queueRows * kRowSpacing + 35);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#f5f8f7"));
    p.drawRoundedRect(queueArea, 15, 15);
    label(p, QRectF(198, 110, 300, 24), tr("FILE D’ATTENTE · TÊTE À GAUCHE"), 10, kMuted, true,
          Qt::AlignLeft | Qt::AlignVCenter);
    for (int row = 0; row < m_queueRows; ++row)
    {
        p.setPen(QPen(QColor("#cbdcd5"), 1.3, Qt::DashLine));
        const double lineY = kQueueFeet + 19 + row * kRowSpacing;
        p.drawLine(QPointF(210, lineY), QPointF(934, lineY));
        for (int column = 0; column < kColumns; ++column)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#dde8e3"));
            p.drawEllipse(QPointF(226 + column * 98, lineY + 3), 2, 2);
        }
    }

    // Porte de sortie : les personnages passent au-delà du bord avant suppression.
    const double exitY = cashierTop() + 165;
    p.setPen(QPen(QColor("#eff8f5"), 9));
    p.drawLine(QPointF(1060, exitY - 30), QPointF(1060, exitY + 24));
    p.setPen(QPen(QColor("#77b5a6"), 3));
    p.drawLine(QPointF(1060, exitY - 30), QPointF(1036, exitY - 45));
    label(p, QRectF(975, exitY - 68, 78, 20), tr("SORTIE →"), 11, kTeal, true);

    // Deux petites plantes, formes vectorielles discrètes.
    for (QPointF pot : {QPointF(145, 131), QPointF(1017, 132)})
    {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#d9e5df"));
        p.drawRoundedRect(QRectF(pot.x() - 9, pot.y(), 18, 16), 4, 4);
        p.setBrush(QColor("#8fb5a0"));
        p.drawEllipse(pot + QPointF(-5, -8), 6, 13);
        p.setBrush(QColor("#579982"));
        p.drawEllipse(pot + QPointF(5, -10), 6, 15);
    }
}

QString BankScene::arrivalLabel(const ClientVisual& visual) const
{
    return tr("Arrivée : %1 u.t.").arg(visual.model->getArrivalTime());
}

QString BankScene::patienceLabel(const ClientVisual& visual) const
{
    // Le nombre est exact ; seule la largeur de la barre est interpolée.
    return tr("Patience : %1 u.t.").arg(visual.model->getPatienceTime());
}

QString BankScene::cashierDetail(int index) const
{
    const auto& cashier = m_cashiers[index];
    if (!cashier.client) return tr("Prêt à vous accueillir");
    const auto it = m_index.find(cashier.client);
    if (it != m_index.end() && it->second->state != VisualState::BeingServed)
        return tr("EN APPROCHE");
    return tr("Reste %1 u.t.").arg(cashier.remaining);
}

void BankScene::drawCashier(QPainter& p, int index) const
{
    const auto& cashier = m_cashiers[index];
    const double x = servicePosition(index).x();
    const double y = cashierTop();
    const QRectF card(x - 106, y, 212, 170);
    p.setPen(QPen(QColor("#dbe6e1"), 1));
    p.setBrush(QColor("#fbfcfc"));
    p.drawRoundedRect(card, 14, 14);
    label(p, QRectF(x - 95, y + 12, 190, 20), tr("GUICHET %1").arg(index + 1), 11, kInk, true);
    person(p, QPointF(x, y + 106), QColor("#78978f"), 0, false);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#cdded7"));
    p.drawRoundedRect(QRectF(x - 78, y + 91, 156, 12), 4, 4);
    p.setBrush(QColor("#e6eeea"));
    p.drawRoundedRect(QRectF(x - 73, y + 103, 146, 12), 3, 3);
    // Petit terminal posé sur le comptoir.
    p.setBrush(QColor("#526e68"));
    p.drawRoundedRect(QRectF(x + 35, y + 73, 21, 17), 3, 3);
    p.setBrush(QColor("#b9d9cb"));
    p.drawRoundedRect(QRectF(x + 38, y + 76, 15, 10), 2, 2);

    const bool busy = cashier.client != nullptr;
    label(p, QRectF(x - 85, y + 119, 170, 20), busy ? tr("OCCUPÉ") : tr("LIBRE"),
          11, busy ? kOrange : kTeal, true);
    label(p, QRectF(x - 98, y + 140, 196, 18), cashierDetail(index), 10, kMuted);
    if (busy)
    {
        const double done = std::clamp(1.0 - double(cashier.remaining) / std::max(1, cashier.total), 0.0, 1.0);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#e0eae5"));
        p.drawRoundedRect(QRectF(x - 71, y + 164, 142, 3), 1.5, 1.5);
        p.setBrush(kOrange);
        p.drawRoundedRect(QRectF(x - 71, y + 164, 142 * done, 3), 1.5, 1.5);
    }
}

void BankScene::drawClient(QPainter& p, const ClientVisual& visual) const
{
    const auto* model = visual.model;
    const bool exhausted = model->getPatienceTime() == 0;
    const bool waiting = visual.modelState == ModelState::Waiting;
    const bool urgentWaiting = waiting && exhausted && model->getOperation()->isUrgent();
    const QColor color = exhausted ? (urgentWaiting ? kOrange : kRed)
                                  : model->isPriority() ? kGold : kBlue;
    const QPointF pos = visual.position;
    person(p, pos, color, visual.walkPhase, visual.moving);
    if (waiting)
    {
        label(p, QRectF(pos.x() - 48, pos.y() - 135, 96, 16), arrivalLabel(visual), 10, kMuted);
        label(p, QRectF(pos.x() - 48, pos.y() - 119, 96, 16), patienceLabel(visual),
              10, exhausted ? color : kMuted);
    }
    label(p, QRectF(pos.x() - 48, pos.y() - (waiting ? 103 : 88), 96, 17),
          operationLabel(model), 10, kInk);
    if (model->isPriority())
    {
        const double vipY = waiting ? -81 : -99;
        star(p, pos + QPointF(-15, vipY));
        label(p, QRectF(pos.x() - 5, pos.y() + vipY - 8, 31, 16), tr("VIP"), 10, kGold, true);
    }
    if (waiting || visual.state == VisualState::Entering)
    {
        const double fraction = std::clamp(visual.shownPatience / m_maxPatience, 0.0, 1.0);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#e2e9e5"));
        p.drawRoundedRect(QRectF(pos.x() - 25, pos.y() - 67, 50, 4), 2, 2);
        p.setBrush(exhausted ? kRed : fraction < 0.35 ? kGold : kTeal);
        p.drawRoundedRect(QRectF(pos.x() - 25, pos.y() - 67, 50 * fraction, 4), 2, 2);
        if (urgentWaiting)
            label(p, QRectF(pos.x() - 46, pos.y() + 7, 92, 17), tr("Urgent · reste"), 9, kOrange, true);
    }
    if (visual.modelState == ModelState::Serving && visual.state != VisualState::BeingServed)
        label(p, QRectF(pos.x() - 48, pos.y() + 8, 96, 17), tr("EN APPROCHE"), 9, kOrange, true);
    if (visual.modelState == ModelState::Abandoned)
        label(p, QRectF(pos.x() - 47, pos.y() + 8, 94, 17), tr("Abandon"), 10, kRed, true);
    else if (visual.modelState == ModelState::Served)
        label(p, QRectF(pos.x() - 47, pos.y() + 8, 94, 17), tr("Service terminé"), 9, kTeal);
}
