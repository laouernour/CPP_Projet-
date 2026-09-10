#include <QtTest>
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QComboBox>
#include <QTemporaryDir>
#include <QTabWidget>
#include <algorithm>
#include <cmath>
#include <set>
#include "BankScene.h"
#include "MainWindow.h"
#include "StatisticsView.h"
#include "database/Database.h"
#include "client/Client.h"
#include "client/Consultation.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationDefaults.h"

class BankSceneTest : public QObject
{
    Q_OBJECT
    static bool settle(BankScene& scene)
    {
        for (int i = 0; i < 3000 && scene.hasPendingTransitions(); ++i) scene.advance(1.0 / 60.0);
        return !scene.hasPendingTransitions();
    }
private slots:
    void chartsReadSavedResultsAndPreserveLiveData()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("statistics.db");
        StatisticsView view(path);
        view.record(0, 0, 0);
        view.record(1, 2, 1);
        view.record(1, 3, 1);
        QCOMPARE(view.m_live.size(), std::size_t(2));
        view.reloadHistory();
        QCOMPARE(view.m_source->count(), 1);
        QVERIFY(!QFileInfo::exists(path));
        Simulation simulation(SimulationEntry(40, 1, 4, 4, 1, 0.0, 1));
        simulation.setVerbose(false);
        while (!simulation.isFinished()) simulation.step();
        {
            Database database(path.toUtf8().toStdString());
            database.saveSimulation(simulation.getEntry(), simulation.getStatistics());
        }
        view.reloadHistory();
        QCOMPARE(view.m_source->count(), 2);
        view.m_source->setCurrentIndex(1);
        QVERIFY(!view.m_chart->m_samples.empty());
        const auto saved = view.m_chart->m_samples.back();
        QCOMPARE(saved.time, 40);
        QCOMPARE(saved.served, simulation.getStatistics().servedClientCount());
        QCOMPARE(saved.abandoned, simulation.getStatistics().nonServedClientCount());
        QVERIFY(view.m_note->text().contains("bank2.db"));
        view.record(2, 4, 2);
        QCOMPARE(view.m_chart->m_samples.back().served, saved.served);
        view.resize(1000, 600);
        view.show();
        view.grab().save("bank-statistics-history.png");
        view.m_source->setCurrentIndex(0);
        QCOMPARE(view.m_chart->m_samples.back().served, 4);
        QCOMPARE(view.m_chart->m_samples.back().time, 2);
        MainWindow window;
        window.show();
        auto* tabs = window.findChild<QTabWidget*>("simulationTabs");
        QVERIFY(tabs);
        window.m_simulationTimer->stop();
        window.m_scene->m_animationTimer->stop();
        for (int i = 0; i < 40; ++i) window.onSimulationTick();
        tabs->setCurrentIndex(1);
        window.grab().save("bank-statistics-live.png");
    }

    void waitingInformationAndApproach()
    {
        Simulation simulation(SimulationEntry(40, 1, 20, 20, 1, 1.0, 100));
        simulation.setVerbose(false);
        BankScene scene;
        scene.m_animationTimer->stop();
        simulation.step();
        scene.syncWith(simulation);
        QCOMPARE(scene.cashierDetail(0), QStringLiteral("EN APPROCHE"));
        simulation.step();
        scene.syncWith(simulation);
        const auto* client = simulation.getBank().getQueue().items().front();
        auto* visual = scene.m_index.at(client);
        QVERIFY(client->isPriority());
        QCOMPARE(visual->state, BankScene::VisualState::Entering);
        QCOMPARE(scene.arrivalLabel(*visual), QStringLiteral("Arrivée : 1 u.t."));
        QCOMPARE(scene.patienceLabel(*visual), QStringLiteral("Patience : 100 u.t."));
        QVERIFY(settle(scene));
        QCOMPARE(visual->state, BankScene::VisualState::Waiting);
        QCOMPARE(visual->position, scene.queuePosition(0));
        QCOMPARE(scene.cashierDetail(0), QStringLiteral("Reste 19 u.t."));
        simulation.step();
        scene.syncWith(simulation);
        QCOMPARE(scene.arrivalLabel(*visual), QStringLiteral("Arrivée : 1 u.t."));
        QCOMPARE(scene.patienceLabel(*visual), QStringLiteral("Patience : 99 u.t."));
        QVERIFY(settle(scene));
        scene.resize(1120, 680);
        scene.show();
        QCoreApplication::processEvents();
        QVERIFY(scene.grab().save(QStringLiteral("bank-vip-details.png")));
    }

    void targetNeverOvershot()
    {
        Client client(0, new Consultation(5), 7);
        BankScene scene;
        scene.m_animationTimer->stop();
        auto* visual = scene.ensureVisual(&client);
        for (double distance : {0.0, 0.1, 0.75, 1.1, 2.2, 4.5})
        {
            visual->position = QPointF(100 - distance, 100);
            visual->state = BankScene::VisualState::JoiningQueue;
            scene.setRoute(*visual, {QPointF(100, 100)});
            scene.advance(1.0 / 60.0);
            QCOMPARE(visual->position, QPointF(100, 100));
            QVERIFY(!visual->moving);
            for (int frame = 0; frame < 10; ++frame) scene.advance(1.0 / 60.0);
            QCOMPARE(visual->position, QPointF(100, 100));
        }
        // La même durée donne la même distance à 30 et 60 FPS.
        visual->position = QPointF(0, 0);
        scene.setRoute(*visual, {QPointF(1000, 0)});
        for (int i = 0; i < 30; ++i) scene.advance(1.0 / 30.0);
        const QPointF thirty = visual->position;
        visual->position = QPointF(0, 0);
        scene.setRoute(*visual, {QPointF(1000, 0)});
        for (int i = 0; i < 60; ++i) scene.advance(1.0 / 60.0);
        QVERIFY(std::abs(visual->position.x() - thirty.x()) < 1e-8);
        QVERIFY(std::abs(thirty.x() - 285.0) < 1e-8);
    }

    void realEngineLifecycle()
    {
        Simulation simulation(SimulationEntry(45, 1, 5, 5, 1, 1.0, 100));
        simulation.setVerbose(false);
        BankScene scene;
        scene.m_animationTimer->stop();
        scene.syncWith(simulation);
        QVERIFY(scene.m_visuals.empty());
        while (!simulation.isFinished())
        {
            simulation.step();
            scene.syncWith(simulation);
            for (const auto* model : simulation.getBank().getQueue().items())
            {
                QVERIFY(scene.m_index.count(model));
                QCOMPARE(scene.m_index.at(model)->modelState, BankScene::ModelState::Waiting);
            }
            for (const auto* cashier : simulation.getBank().getCashiers())
                if (const auto* model = cashier->getServingClient())
                {
                    QVERIFY(scene.m_index.count(model));
                    QCOMPARE(scene.m_index.at(model)->modelState, BankScene::ModelState::Serving);
                }
            QVERIFY(settle(scene));
            for (const auto& visual : scene.m_visuals)
                if (visual->modelState == BankScene::ModelState::Serving)
                {
                    QCOMPARE(visual->state, BankScene::VisualState::BeingServed);
                    QCOMPARE(visual->position, scene.servicePosition(visual->cashierIndex));
                    QVERIFY(!visual->moving);
                }
        }
        for (int i = 0; i < 1000; ++i) scene.advance(1.0 / 60.0);
        const auto count = scene.m_visuals.size();
        scene.syncWith(simulation);
        QCOMPARE(scene.m_visuals.size(), count); // Aucun client sorti ne réapparaît.
        QCOMPARE(scene.m_seen.size(), std::size_t(46));
    }

    void patienceAndDeparturesFollowEngine()
    {
        Simulation simulation(SimulationEntry(40, 1, 20, 20, 1, 0.0, 1));
        simulation.setVerbose(false);
        BankScene scene;
        scene.m_animationTimer->stop();
        std::size_t departuresSeen = 0;
        while (!simulation.isFinished())
        {
            std::unordered_map<const AbstractClient*, QPointF> previous;
            for (const auto& visual : scene.m_visuals) previous[visual->model] = visual->position;
            simulation.step();
            scene.syncWith(simulation);
            for (const auto& item : previous)
                QCOMPARE(scene.m_index.at(item.first)->position, item.second); // Aucun saut au sync.
            for (const auto* client : simulation.getBank().getQueue().items())
            {
                const auto* visual = scene.m_index.at(client);
                QCOMPARE(visual->targetPatience, double(client->getPatienceTime()));
                if (client->getPatienceTime() == 0)
                {
                    QVERIFY(client->getOperation()->isUrgent());
                    QCOMPARE(visual->shownPatience, 0.0);
                    QCOMPARE(visual->modelState, BankScene::ModelState::Waiting);
                }
            }
            const auto& abandoned = simulation.getStatistics().nonServedClients();
            while (departuresSeen < abandoned.size())
            {
                const auto* client = abandoned[departuresSeen++];
                const auto* visual = scene.m_index.at(client);
                QCOMPARE(visual->modelState, BankScene::ModelState::Abandoned);
                QCOMPARE(visual->state, BankScene::VisualState::Leaving);
                QCOMPARE(visual->shownPatience, 0.0);
                QVERIFY(!client->getOperation()->isUrgent());
            }
            QVERIFY(settle(scene));
        }
    }

    void longQueueHasUniquePlaces()
    {
        Simulation simulation(SimulationEntry(30, 3, 100, 100, 1, 1.0, 100));
        simulation.setVerbose(false);
        BankScene scene;
        scene.m_animationTimer->stop();
        while (!simulation.isFinished())
        {
            simulation.step();
            scene.syncWith(simulation);
            QVERIFY(settle(scene));
        }
        QCOMPARE(simulation.getBank().getQueue().size(), 28);
        QVERIFY(scene.m_queueRows >= 4);
        std::set<std::pair<double, double>> positions;
        for (const auto& visual : scene.m_visuals)
            QVERIFY(positions.emplace(visual->position.x(), visual->position.y()).second);
        QCOMPARE(positions.size(), std::size_t(31));
        const QPointF before = scene.m_visuals.front()->position;
        scene.resize(950, 620);
        QCOMPARE(scene.m_visuals.front()->position, before);
        QVERIFY(scene.minimumHeight() <= 620);
        for (const QSize size : {QSize(950, 620), QSize(640, 280), QSize(1200, 800)})
        {
            scene.resize(size);
            QVERIFY(1120.0 * scene.sceneScale() <= scene.width() + 0.001);
            QVERIFY(scene.canvasHeight() * scene.sceneScale() <= scene.height() + 0.001);
            QCOMPARE(scene.m_visuals.front()->position, before);
        }
    }

    void oneButtonStartsAndPausesBothClocks()
    {
        MainWindow window;
        window.show();
        QCOMPARE(window.findChildren<QPushButton*>().size(), 1);
        QCOMPARE(window.findChildren<QTimer*>().size(), 2);
        QCOMPARE(window.m_simulation->getEntry().getSimulationDuration(), defaultSimulationEntry().getSimulationDuration());
        QCOMPARE(window.m_simulation->getEntry().getClientArrivalInterval(), defaultSimulationEntry().getClientArrivalInterval());
        QTest::mouseClick(window.m_runButton, Qt::LeftButton);
        QCOMPARE(window.m_simulation->getCurrentTime(), 1);
        QCOMPARE(window.m_scene->m_visuals.size(), std::size_t(1));
        auto* visual = window.m_scene->m_visuals.front().get();
        QCOMPARE(visual->state, BankScene::VisualState::Entering);
        QCOMPARE(visual->position, QPointF(30, 200));
        window.onSimulationTick();
        QCOMPARE(window.m_simulation->getCurrentTime(), 2); // Avance même pendant la marche.
        QTest::qWait(80);
        QVERIFY(visual->position != QPointF(30, 200));
        QTest::mouseClick(window.m_runButton, Qt::LeftButton);
        const QPointF pausedPosition = visual->position;
        QTest::qWait(100);
        QCOMPARE(visual->position, pausedPosition);
        QCOMPARE(window.m_simulation->getCurrentTime(), 2);
        QVERIFY(!window.m_simulationTimer->isActive());
        QVERIFY(window.m_clock->text().contains(QStringLiteral("1 / %1")
            .arg(window.m_simulation->getEntry().getSimulationDuration())));
        QVERIFY(window.m_clock->text().startsWith(QStringLiteral("Temps : ")));
        QVERIFY(window.m_clock->text().endsWith(QStringLiteral(" u.t.")));
        QVERIFY(window.m_metrics->text().contains(QStringLiteral("Attente moyenne :")));
        QVERIFY(window.m_metrics->text().contains(QStringLiteral("Occupation :")));
        QVERIFY(window.m_metrics->text().contains(QStringLiteral("Satisfaction :")));
    }

    void clientsStayDistinctDuringAnimation()
    {
        Simulation simulation(SimulationEntry(40, 3, 10, 10, 2, 1.0, 40));
        simulation.setVerbose(false);
        BankScene scene;
        scene.m_animationTimer->stop();
        while (!simulation.isFinished())
        {
            simulation.step();
            scene.syncWith(simulation);
            for (int frame = 0; frame < 39; ++frame)
            {
                scene.advance(1.0 / 60.0);
                for (std::size_t i = 0; i < scene.m_visuals.size(); ++i)
                    for (std::size_t j = i + 1; j < scene.m_visuals.size(); ++j)
                    {
                        const QPointF distance = scene.m_visuals[i]->position - scene.m_visuals[j]->position;
                        QVERIFY(std::hypot(distance.x(), distance.y()) > 0.001);
                    }
            }
        }
        QVERIFY(settle(scene));
    }

    void renderingNeverControlsTheEngine()
    {
        MainWindow window;
        // Aucun rendu entre les pas : le moteur doit malgré tout progresser.
        window.m_scene->m_animationTimer->stop();
        window.toggleRun();
        window.m_simulationTimer->stop();
        const auto* first = window.m_scene->m_visuals.front()->model;
        const QPointF initialPosition = window.m_scene->m_visuals.front()->position;
        for (int expected = 2; expected <= 25; ++expected)
        {
            window.onSimulationTick();
            QCOMPARE(window.m_simulation->getCurrentTime(), expected);
        }
        const auto& stats = window.m_simulation->getStatistics();
        QVERIFY(std::find(stats.servedClients().begin(), stats.servedClients().end(), first)
                != stats.servedClients().end());
        auto* visual = window.m_scene->m_index.at(first);
        QCOMPARE(visual->position, initialPosition);
        QCOMPARE(visual->modelState, BankScene::ModelState::Served);

        const int time = window.m_simulation->getCurrentTime();
        const int served = stats.servedClientCount();
        const int abandoned = stats.nonServedClientCount();
        const int waiting = window.m_simulation->getBank().getQueue().size();
        // Le rattrapage visuel ne modifie aucune donnée métier.
        for (int frame = 0; frame < 1500; ++frame) window.m_scene->advance(1.0 / 60.0);
        QCOMPARE(window.m_simulation->getCurrentTime(), time);
        QCOMPARE(stats.servedClientCount(), served);
        QCOMPARE(stats.nonServedClientCount(), abandoned);
        QCOMPARE(window.m_simulation->getBank().getQueue().size(), waiting);
        QVERIFY(window.m_scene->m_index.count(first) == 0);
    }

    void finalStateAndScreenshots()
    {
        MainWindow window;
        window.show();
        window.m_scene->m_animationTimer->stop();
        QCoreApplication::processEvents();
        const QString output = QDir::currentPath();
        QVERIFY(window.grab().save(output + QStringLiteral("/bank-ready.png")));
        // Même moteur et mêmes paramètres que l'application ; avance accélérée
        // seulement dans le test pour contrôler l'état final et son rendu.
        window.toggleRun();
        window.m_simulationTimer->stop();
        bool screenshot = false;
        while (!window.m_simulation->isFinished())
        {
            QVERIFY(settle(*window.m_scene));
            window.onSimulationTick();
            if (!screenshot && window.m_simulation->getCurrentTime() >= 42)
            {
                QVERIFY(settle(*window.m_scene));
                QCoreApplication::processEvents();
                QVERIFY(window.grab().save(output + QStringLiteral("/bank-running.png")));
                screenshot = true;
            }
        }
        QVERIFY(settle(*window.m_scene));
        for (int i = 0; i < 1000; ++i) window.m_scene->advance(1.0 / 60.0);
        QVERIFY(!window.m_simulationTimer->isActive());
        QVERIFY(!window.m_runButton->isEnabled());
        QVERIFY(window.m_savedToDb);
        const auto& entry = window.m_simulation->getEntry();
        QCOMPARE(window.m_clock->text(), QStringLiteral("Temps : %1 / %1 u.t.")
            .arg(entry.getSimulationDuration()));
        const int arrivals = entry.getSimulationDuration() / entry.getClientArrivalInterval() + 1;
        QCOMPARE(window.m_values[0]->text(), QString::number(arrivals));
        QCoreApplication::processEvents();
        QVERIFY(window.grab().save(output + QStringLiteral("/bank-finished.png")));
    }
};

int main(int argc, char** argv)
{
    QApplication application(argc, argv);
    // Le plugin offscreen ne consulte pas automatiquement les polices Windows.
    const QString fonts = qEnvironmentVariable("WINDIR") + QStringLiteral("/Fonts/");
    for (const QString& file : {QStringLiteral("segoeui.ttf"), QStringLiteral("segoeuib.ttf"),
                                QStringLiteral("seguisb.ttf")})
        QFontDatabase::addApplicationFont(fonts + file);
    application.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    application.setStyle(QStringLiteral("Fusion"));
    BankSceneTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "BankSceneTest.moc"
