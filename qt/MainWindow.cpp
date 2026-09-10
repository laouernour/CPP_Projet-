#include "MainWindow.h"

#include <algorithm>
#include <exception>
#include <QDir>
#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QTabWidget>
#include <QVBoxLayout>
#include "BankScene.h"
#include "StatisticsView.h"
#include "bank/Bank.h"
#include "bank/Cashier.h"
#include "bank/Queue.h"
#include "database/Database.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationDefaults.h"
#include "simulation/StatisticManager.h"

namespace
{
QIcon actionIcon(bool pause)
{
    // Icône vectorielle : ne dépend pas des glyphes disponibles dans la police.
    QPixmap image(18, 18);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    if (pause)
    {
        painter.drawRoundedRect(QRectF(4, 3, 3, 12), 1, 1);
        painter.drawRoundedRect(QRectF(11, 3, 3, 12), 1, 1);
    }
    else
    {
        QPolygonF triangle;
        triangle << QPointF(5, 3) << QPointF(15, 9) << QPointF(5, 15);
        painter.drawPolygon(triangle);
    }
    return QIcon(image);
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_simulation(std::make_unique<Simulation>(defaultSimulationEntry()))
{
    m_simulation->setVerbose(false);
    setWindowTitle(tr("Simulation Banque"));
    resize(1100, 740);
    setMinimumSize(840, 600);
    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget#central { background: #f3f6f5; color: #203c46; }
        QLabel { background: transparent; font-family: 'Segoe UI'; color: #203c46; }
        QLabel#title { color: #167b72; font-size: 18px; font-weight: 700; }
        QLabel#status, QLabel#clock, QLabel#metrics { color: #788b92; font-size: 12px; }
        QPushButton { background: #167b72; color: white; border: none; border-radius: 17px;
                      padding: 7px 22px; font: 600 12px 'Segoe UI'; min-width: 120px; }
        QPushButton:hover { background: #12685f; }
        QPushButton:pressed { background: #10584f; }
        QPushButton:disabled { background: #dae6e1; color: #728880; }
        QPushButton:focus { border: 2px solid #8ec8b9; }
        QFrame#statCard { background: white; border: 1px solid #e0e8e4; border-radius: 12px; }
        QLabel#statValue { font-size: 22px; font-weight: 700; }
        QLabel#statName { color: #788b92; font-size: 11px; }
    )"));

    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("central"));
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(18, 12, 18, 10);
    layout->setSpacing(5);
    auto* header = new QHBoxLayout;
    header->setSpacing(16);
    auto* title = new QLabel(tr("Simulation de banque"), central);
    title->setObjectName(QStringLiteral("title"));
    header->addStretch();
    header->addWidget(title);
    m_runButton = new QPushButton(tr("Démarrer"), central);
    m_runButton->setIcon(actionIcon(false));
    m_runButton->setObjectName(QStringLiteral("runButton"));
    m_runButton->setCursor(Qt::PointingHandCursor);
    header->addWidget(m_runButton);
    header->addStretch();
    layout->addLayout(header);
    connect(m_runButton, &QPushButton::clicked, this, &MainWindow::toggleRun);

    m_scene = new BankScene(central);
    auto* tabs = new QTabWidget(central);
    tabs->setObjectName(QStringLiteral("simulationTabs"));
    tabs->addTab(m_scene, tr("Animation"));
    const QString databasePath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bank2.db"));
    m_statisticsView = new StatisticsView(databasePath, tabs);
    tabs->addTab(m_statisticsView, tr("Statistiques"));
    layout->addWidget(tabs, 1);
    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1) m_statisticsView->reloadHistory();
    });

    auto* stats = new QHBoxLayout;
    stats->setSpacing(8);
    const std::array<QString, 4> names{tr("Clients arrivés"), tr("File actuelle"), tr("Clients servis"), tr("Clients non servis")};
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        auto* card = new QFrame(central);
        card->setObjectName(QStringLiteral("statCard"));
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 6, 14, 7);
        cardLayout->setSpacing(0);
        m_values[i] = new QLabel(QStringLiteral("0"), card);
        m_values[i]->setObjectName(QStringLiteral("statValue"));
        auto* name = new QLabel(names[i], card);
        name->setObjectName(QStringLiteral("statName"));
        cardLayout->addWidget(m_values[i]);
        cardLayout->addWidget(name);
        if (i == 3) card->setToolTip(tr("Clients ayant abandonné selon le moteur."));
        stats->addWidget(card);
    }
    layout->addLayout(stats);
    m_metrics = new QLabel(central);
    m_metrics->setObjectName(QStringLiteral("metrics"));
    m_metrics->setAlignment(Qt::AlignCenter);
    m_metrics->setWordWrap(true);
    m_metrics->setToolTip(tr("Attente moyenne des clients servis ; indicateurs calculés par StatisticManager."));
    layout->addWidget(m_metrics);
    auto* footer = new QHBoxLayout;
    m_status = new QLabel(tr("Prêt à démarrer"), central);
    m_status->setObjectName(QStringLiteral("status"));
    m_status->setTextFormat(Qt::PlainText);
    m_clock = new QLabel(central);
    m_clock->setObjectName(QStringLiteral("clock"));
    footer->addWidget(m_status, 1);
    footer->addWidget(m_clock);
    layout->addLayout(footer);
    setCentralWidget(central);

    m_simulationTimer = new QTimer(this);
    m_simulationTimer->setObjectName(QStringLiteral("simulationTimer"));
    m_simulationTimer->setTimerType(Qt::PreciseTimer);
    m_simulationTimer->setInterval(650);
    connect(m_simulationTimer, &QTimer::timeout, this, &MainWindow::onSimulationTick);
    m_scene->syncWith(*m_simulation);
    refreshStats();
}

MainWindow::~MainWindow()
{
    // La scène emprunte des pointeurs du moteur : la détruire avant les clients.
    m_simulationTimer->stop();
    delete m_scene;
    m_scene = nullptr;
}

void MainWindow::toggleRun()
{
    if (m_simulation->isFinished()) return;
    if (m_simulationTimer->isActive())
    {
        m_simulationTimer->stop();
        m_scene->setPaused(true);
        m_runButton->setText(tr("Reprendre"));
        m_runButton->setIcon(actionIcon(false));
        m_status->setText(tr("En pause"));
        return;
    }
    m_scene->setPaused(false);
    m_runButton->setText(tr("Pause"));
    m_runButton->setIcon(actionIcon(true));
    m_status->setText(tr("La banque est ouverte"));
    m_simulationTimer->start();
    if (!m_started)
    {
        m_started = true;
        onSimulationTick(); // Premier vrai client dès le clic.
    }
}

void MainWindow::onSimulationTick()
{
    if (m_simulation->isFinished()) return;
    // La cadence du moteur ne dépend jamais de la progression des personnages.
    // La scène rejoint visuellement le dernier état reçu, sans bloquer step().
    const bool running = m_simulation->step();
    m_scene->syncWith(*m_simulation);
    refreshStats();
    if (!running)
    {
        m_simulationTimer->stop();
        m_runButton->setText(tr("Simulation terminée"));
        m_runButton->setIcon(QIcon());
        m_runButton->setEnabled(false);
        m_status->setText(tr("Simulation terminée"));
        saveResults();
        // Le timer graphique termine les trajets. Les clients encore en service
        // ou en file à l'horizon restent visibles, conformément au moteur.
    }
}

void MainWindow::refreshStats()
{
    const auto& stats = m_simulation->getStatistics();
    const auto& bank = m_simulation->getBank();
    const int waiting = bank.getQueue().size();
    const int served = stats.servedClientCount();
    const int abandoned = stats.nonServedClientCount();
    int active = 0;
    for (const auto* cashier : bank.getCashiers()) if (!cashier->isFree()) ++active;
    const std::array<int, 4> values{served + abandoned + waiting + active, waiting, served, abandoned};
    for (std::size_t i = 0; i < values.size(); ++i) m_values[i]->setText(QString::number(values[i]));
    m_metrics->setText(tr("Attente moyenne : %1 u.t.   ·   Occupation : %2 %   ·   Satisfaction : %3 %")
        .arg(stats.calculateAverageClientWaitingTime(), 0, 'f', 2)
        .arg(stats.calculateAverageCashierOccupationRate(m_simulation->getEntry().getCashierCount()), 0, 'f', 1)
        .arg(stats.calculateClientSatisfactionRate(), 0, 'f', 1));
    // getCurrentTime() désigne le PROCHAIN pas. On affiche l'instant traité,
    // sans changer la convention 0..durée du moteur ni ses tests.
    const int duration = m_simulation->getEntry().getSimulationDuration();
    const int processedTime = m_simulation->isFinished() ? duration
        : std::clamp(m_simulation->getCurrentTime() - 1, 0, duration);
    m_clock->setText(tr("Temps : %1 / %2 u.t.").arg(processedTime).arg(duration));
    m_statisticsView->record(processedTime, served, abandoned);
}

void MainWindow::saveResults()
{
    if (m_savedToDb) return;
    try
    {
        // Fichier stable à côté de l'exécutable, indépendant du dossier de lancement.
        const QString path = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bank2.db"));
        Database database(path.toUtf8().toStdString());
        database.saveSimulation(m_simulation->getEntry(), m_simulation->getStatistics());
        m_savedToDb = true; // Seulement après la réussite de la transaction.
        m_statisticsView->reloadHistory();
        m_status->setText(tr("Simulation terminée · Résultats enregistrés"));
        m_status->setToolTip(QDir::toNativeSeparators(path));
    }
    catch (const std::exception& error)
    {
        m_status->setText(tr("Simulation terminée · Enregistrement indisponible"));
        m_status->setToolTip(QString::fromUtf8(error.what()));
    }
}
