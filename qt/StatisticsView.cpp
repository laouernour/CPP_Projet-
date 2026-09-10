#include "StatisticsView.h"

#include <QComboBox>
#include <QFileInfo>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include "database/sqlite/sqlite3.h"

namespace
{
const QColor servedColor("#167b72"), abandonedColor("#dc5657"), ink("#203c46");
using Connection = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>;
using Statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

Connection openHistory(const QString& path)
{
    sqlite3* raw = nullptr;
    const int rc = sqlite3_open_v2(path.toUtf8().constData(), &raw, SQLITE_OPEN_READONLY, nullptr);
    Connection db(raw, sqlite3_close);
    if (rc != SQLITE_OK) throw std::runtime_error("Impossible de lire la base de statistiques.");
    return db;
}

Statement query(sqlite3* db, const char* sql)
{
    sqlite3_stmt* raw = nullptr;
    const int rc = sqlite3_prepare_v2(db, sql, -1, &raw, nullptr);
    Statement statement(raw, sqlite3_finalize);
    if (rc != SQLITE_OK) throw std::runtime_error(sqlite3_errmsg(db));
    return statement;
}
}

StatisticsChart::StatisticsChart(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(640, 280);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void StatisticsChart::setSamples(const std::vector<StatisticsSample>& samples)
{
    m_samples = samples;
    update();
}

void StatisticsChart::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const double scale = std::min(width() / 1000.0, height() / 530.0);
    p.translate((width() - 1000 * scale) / 2, (height() - 530 * scale) / 2);
    p.scale(scale, scale);
    QFont font("Segoe UI");
    font.setPixelSize(13);
    p.setFont(font);
    auto text = [&](QRectF rect, const QString& value) {
        p.setPen(ink); p.drawText(rect, Qt::AlignCenter, value);
    };
    p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    p.drawRoundedRect(QRectF(5, 5, 485, 240), 12, 12);
    p.drawRoundedRect(QRectF(505, 5, 490, 240), 12, 12);
    p.drawRoundedRect(QRectF(5, 255, 990, 270), 12, 12);
    text(QRectF(20, 12, 450, 25), tr("Nombre de clients ayant quitté la banque"));
    text(QRectF(520, 12, 450, 25), tr("Répartition des départs"));
    text(QRectF(20, 263, 960, 25), tr("Évolution des départs cumulés · nombre de clients"));
    const auto last = m_samples.empty() ? StatisticsSample{} : m_samples.back();
    const int maximum = std::max({1, last.served, last.abandoned});
    const std::array<int, 2> values{last.served, last.abandoned};
    const std::array<QColor, 2> colors{servedColor, abandonedColor};
    const std::array<QString, 2> names{tr("Servis"), tr("Abandons")};
    for (int i = 0; i < 2; ++i)
    {
        const double h = 130.0 * values[i] / maximum;
        p.setPen(Qt::NoPen); p.setBrush(colors[i]);
        p.drawRect(QRectF(115 + i * 180, 200 - h, 75, h));
        text(QRectF(90 + i * 180, 174 - h, 125, 23), QString::number(values[i]));
        text(QRectF(90 + i * 180, 204, 125, 23), names[i]);
    }
    const double total = double(last.served) + last.abandoned;
    const QRectF pie(535, 52, 164, 164);
    p.setPen(Qt::NoPen);
    if (total == 0)
    {
        p.setBrush(QColor("#e0e8e4")); p.drawEllipse(pie);
        text(pie, tr("Aucun départ"));
    }
    else
    {
        const int angle = qRound(5760.0 * last.served / total);
        p.setBrush(servedColor); p.drawPie(pie, 90 * 16, angle);
        p.setBrush(abandonedColor); p.drawPie(pie, 90 * 16 + angle, 5760 - angle);
    }
    for (int i = 0; i < 2; ++i)
    {
        p.setPen(Qt::NoPen); p.setBrush(colors[i]);
        p.drawEllipse(QRectF(722, 91 + 46 * i, 10, 10));
        text(QRectF(735, 79 + 46 * i, 248, 34),
             tr("%1 : %2 (%3 %)").arg(names[i]).arg(values[i])
             .arg(total > 0 ? 100.0 * values[i] / total : 0, 0, 'f', 1));
    }
    const QRectF plot(65, 310, 900, 150);
    const double yMaximum = std::ceil(maximum / 4.0) * 4;
    const double xMaximum = std::max(4.0, std::ceil(last.time / 4.0) * 4);
    for (int i = 0; i <= 4; ++i)
    {
        const double y = plot.bottom() - plot.height() * i / 4;
        p.setPen(QColor("#e0e8e4")); p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        text(QRectF(7, y - 10, 52, 20), QString::number(yMaximum * i / 4, 'f', 0));
        const double x = plot.left() + plot.width() * i / 4;
        text(QRectF(x - 30, 465, 60, 20), QString::number(xMaximum * i / 4, 'f', 0));
    }
    for (int series = 0; series < 2; ++series)
    {
        QPainterPath line;
        bool first = true;
        for (const auto& sample : m_samples)
        {
            const QPointF point(plot.left() + plot.width() * sample.time / xMaximum,
                                plot.bottom() - plot.height() * (series == 0 ? sample.served : sample.abandoned) / yMaximum);
            if (first) { line.moveTo(point); first = false; }
            else { line.lineTo(QPointF(point.x(), line.currentPosition().y())); line.lineTo(point); }
        }
        p.setPen(QPen(colors[series], 2.5)); p.setBrush(Qt::NoBrush); p.drawPath(line);
        if (!m_samples.empty()) p.drawEllipse(line.currentPosition(), 2, 2);
    }
    text(QRectF(280, 491, 450, 23), tr("Temps (u.t.) · Vert : servis · Rouge : abandons"));
}

StatisticsView::StatisticsView(const QString& path, QWidget* parent)
    : QWidget(parent), m_databasePath(path)
{
    auto* layout = new QVBoxLayout(this);
    m_source = new QComboBox(this);
    m_source->addItem(tr("Simulation en cours"), QVariant::fromValue<qlonglong>(0));
    m_source->setAccessibleName(tr("Simulation à afficher"));
    layout->addWidget(m_source);
    m_chart = new StatisticsChart(this);
    layout->addWidget(m_chart, 1);
    m_note = new QLabel(this);
    m_note->setWordWrap(true);
    layout->addWidget(m_note);
    connect(m_source, &QComboBox::currentIndexChanged, this, [this] { selectSource(); });
    selectSource();
}

void StatisticsView::record(int time, int served, int abandoned)
{
    const StatisticsSample sample{time, served, abandoned};
    if (!m_live.empty() && m_live.back().time == time) m_live.back() = sample;
    else m_live.push_back(sample);
    if (m_source->currentData().toLongLong() == 0) m_chart->setSamples(m_live);
}

void StatisticsView::reloadHistory()
{
    const auto selected = m_source->currentData().toLongLong();
    const QSignalBlocker blocker(m_source);
    m_source->clear();
    m_source->addItem(tr("Simulation en cours"), QVariant::fromValue<qlonglong>(0));
    try
    {
        if (QFileInfo::exists(m_databasePath))
        {
            auto db = openHistory(m_databasePath);
            auto stmt = query(db.get(), "SELECT s.id,s.started_at,s.duration FROM simulations s JOIN statistics st ON st.simulation_id=s.id ORDER BY s.id DESC");
            int rc;
            while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW)
                m_source->addItem(tr("Simulation #%1 · %2 UTC · %3 u.t.")
                    .arg(sqlite3_column_int64(stmt.get(), 0))
                    .arg(QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1))))
                    .arg(sqlite3_column_int(stmt.get(), 2)),
                    QVariant::fromValue<qlonglong>(sqlite3_column_int64(stmt.get(), 0)));
            if (rc != SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(db.get()));
        }
        m_source->setCurrentIndex(std::max(0, m_source->findData(selected)));
        selectSource();
    }
    catch (const std::exception& e) { m_chart->setSamples(m_live); m_note->setText(tr("Historique indisponible : %1").arg(QString::fromUtf8(e.what()))); }
}

void StatisticsView::selectSource()
{
    const auto id = m_source->currentData().toLongLong();
    if (id == 0)
    {
        m_chart->setSamples(m_live);
        m_note->setText(tr("Données en direct. Le camembert concerne uniquement les clients servis ou ayant abandonné ; les clients encore présents sont exclus."));
        return;
    }
    try
    {
        auto db = openHistory(m_databasePath);
        auto stmt = query(db.get(), "SELECT departure_time,SUM(served),SUM(1-served) FROM clients WHERE simulation_id=? AND departure_time IS NOT NULL GROUP BY departure_time ORDER BY departure_time");
        sqlite3_bind_int64(stmt.get(), 1, id);
        std::vector<StatisticsSample> samples{{0, 0, 0}};
        int rc;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW)
        {
            const auto previous = samples.back();
            samples.push_back({sqlite3_column_int(stmt.get(), 0), previous.served + sqlite3_column_int(stmt.get(), 1),
                               previous.abandoned + sqlite3_column_int(stmt.get(), 2)});
        }
        if (rc != SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(db.get()));
        auto summary = query(db.get(), "SELECT s.duration,st.served_client_count,st.non_served_client_count FROM simulations s JOIN statistics st ON st.simulation_id=s.id WHERE s.id=?");
        sqlite3_bind_int64(summary.get(), 1, id);
        if (sqlite3_step(summary.get()) != SQLITE_ROW) throw std::runtime_error("Simulation introuvable.");
        if (samples.back().served != sqlite3_column_int(summary.get(), 1) || samples.back().abandoned != sqlite3_column_int(summary.get(), 2))
            throw std::runtime_error("Historique des clients incomplet pour tracer la courbe.");
        samples.push_back({sqlite3_column_int(summary.get(), 0), samples.back().served, samples.back().abandoned});
        m_chart->setSamples(samples);
        m_note->setText(tr("Résultats relus depuis bank2.db à partir des départs enregistrés. Clients encore présents exclus. Les compteurs du bas concernent la simulation en cours."));
    }
    catch (const std::exception& e) { m_chart->setSamples({}); m_note->setText(tr("Lecture impossible : %1").arg(QString::fromUtf8(e.what()))); }
}
