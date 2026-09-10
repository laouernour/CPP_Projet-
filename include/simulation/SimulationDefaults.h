#ifndef SIMULATIONDEFAULTS_H
#define SIMULATIONDEFAULTS_H

#include "simulation/SimulationEntry.h"

// Paramètres communs à la console et à Qt.
inline SimulationEntry defaultSimulationEntry()
{
    return SimulationEntry(500, 3, 10, 20, 2, 0.10, 6);
}

#endif
