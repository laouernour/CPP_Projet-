#ifndef BANKSCENE_H
#define BANKSCENE_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <QElapsedTimer>
#include <QPointF>
#include <QWidget>

class AbstractClient;
class QPainter;
class QTimer;
class Simulation;

// Vue seulement : les états métier et les destinations viennent de Simulation.
// Les coordonnées sont exprimées dans un repère logique, indépendant du DPI.
class BankScene : public QWidget
{
    Q_OBJECT
public:
    explicit BankScene(QWidget* parent = nullptr);
    ~BankScene() override;
    void syncWith(const Simulation& simulation);
    void setPaused(bool paused);
    bool hasPendingTransitions() const;

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    friend class BankSceneTest;
    enum class ModelState { Waiting, Serving, Served, Abandoned };
    enum class VisualState { Entering, JoiningQueue, Waiting, GoingToCashier,
                             BeingServed, Leaving, Gone };
    struct ClientVisual
    {
        const AbstractClient* model = nullptr; // Emprunt, Simulation possède le client.
        ModelState modelState = ModelState::Waiting;
        VisualState state = VisualState::Entering;
        QPointF position;
        std::vector<QPointF> route;
        std::size_t waypoint = 0;
        int queueIndex = -1;
        int cashierIndex = -1;
        int arrivalSlot = 0;
        double walkPhase = 0.0;
        double shownPatience = 0.0;
        double targetPatience = 0.0;
        bool moving = false;
    };
    struct CashierSnapshot
    {
        const AbstractClient* client = nullptr;
        int remaining = 0;
        int total = 1;
    };

    ClientVisual* ensureVisual(const AbstractClient* client);
    void setDestination(ClientVisual& visual);
    void setRoute(ClientVisual& visual, std::vector<QPointF> route);
    void finishRoute(ClientVisual& visual);
    void advanceAnimation();
    void advance(double seconds);
    void updateCanvasSize();
    QPointF queuePosition(int index) const;
    QPointF servicePosition(int index) const;
    double cashierTop() const;
    double canvasHeight() const;
    double sceneScale() const;
    QString arrivalLabel(const ClientVisual& visual) const;
    QString patienceLabel(const ClientVisual& visual) const;
    QString cashierDetail(int index) const;
    void drawAgency(QPainter& painter) const;
    void drawCashier(QPainter& painter, int index) const;
    void drawClient(QPainter& painter, const ClientVisual& visual) const;

    std::vector<std::unique_ptr<ClientVisual>> m_visuals;
    std::unordered_map<const AbstractClient*, ClientVisual*> m_index;
    std::unordered_set<const AbstractClient*> m_seen;
    std::vector<CashierSnapshot> m_cashiers;
    std::size_t m_servedSeen = 0;
    std::size_t m_abandonedSeen = 0;
    int m_queueRows = 1;
    int m_maxPatience = 1;
    bool m_paused = false;
    QTimer* m_animationTimer = nullptr;
    QElapsedTimer m_frameClock;
};
#endif
