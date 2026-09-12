#ifndef SIMULATIONDEFAULTS_H
#define SIMULATIONDEFAULTS_H

#include "simulation/SimulationEntry.h"

// Paramètres communs à la console et à Qt.
inline SimulationEntry defaultSimulationEntry()
{
    return SimulationEntry(200, 3, 10, 20,4, 0.10, 9);
}

#endif
