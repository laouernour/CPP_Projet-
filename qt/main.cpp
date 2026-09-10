#include <QApplication>
#include <QFont>
#include "MainWindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Simulation Banque"));
    app.setOrganizationName(QStringLiteral("SimulationBank"));
    app.setStyle(QStringLiteral("Fusion"));
    app.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    MainWindow window;
    window.show();
    return app.exec();
}
