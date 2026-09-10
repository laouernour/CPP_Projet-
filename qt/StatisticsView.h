#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>
#include <array>
#include <vector>

class QComboBox;
class QLabel;

struct StatisticsSample
{
    int time = 0;
    int served = 0;
    int abandoned = 0;
};

class StatisticsChart : public QWidget
{
public:
    explicit StatisticsChart(QWidget* parent = nullptr);
    void setSamples(const std::vector<StatisticsSample>& samples);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    friend class BankSceneTest;
    std::vector<StatisticsSample> m_samples;
};

class StatisticsView : public QWidget
{
public:
    explicit StatisticsView(const QString& databasePath, QWidget* parent = nullptr);
    void record(int time, int served, int abandoned);
    void reloadHistory();
private:
    friend class BankSceneTest;
    void selectSource();
    QString m_databasePath;
    QComboBox* m_source;
    QLabel* m_note;
    StatisticsChart* m_chart;
    std::vector<StatisticsSample> m_live;
};
#endif
