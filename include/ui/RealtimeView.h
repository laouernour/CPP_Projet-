#ifndef REALTIMEVIEW_H
#define REALTIMEVIEW_H

class Simulation;

/**
 * Fenêtre graphique (Win32 / GDI) qui affiche le déroulement de la simulation
 * en temps réel : une unité de temps toutes les `msPerTick` millisecondes.
 *
 * Montre les caissiers (libre / occupé), la file d'attente (clients normaux et
 * VIP) et les statistiques mises à jour en direct.
 */
class RealtimeView
{
public:
    RealtimeView(Simulation& simulation, int msPerTick = 150);

    void run();   // Ouvre la fenêtre et anime la simulation jusqu'à la fin

    Simulation& simulation() const { return m_simulation; }   // Utilisé par la procédure de fenêtre
    int tickInterval() const { return m_msPerTick; }

private:
    Simulation& m_simulation;
    int m_msPerTick;
};

#endif // REALTIMEVIEW_H
