#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>
#include <memory>
#include <QMainWindow>

class BankScene;
class QLabel;
class QPushButton;
class QTimer;
class Simulation;
class StatisticsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    friend class BankSceneTest;
    void toggleRun();
    void onSimulationTick();
    void refreshStats();
    void saveResults();

    std::unique_ptr<Simulation> m_simulation;
    BankScene* m_scene = nullptr;
    StatisticsView* m_statisticsView = nullptr;
    QTimer* m_simulationTimer = nullptr;
    QPushButton* m_runButton = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_clock = nullptr;
    QLabel* m_metrics = nullptr;
    std::array<QLabel*, 4> m_values{};
    bool m_started = false;
    bool m_savedToDb = false;
};
#endif
