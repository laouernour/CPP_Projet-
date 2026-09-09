# bank2_cpp

Simulation d'une banque en C++17 : opérations bancaires polymorphes, clients
(standard / VIP), file d'attente avec priorité et patience, caissiers,
enregistrement des résultats dans une base de données SQLite, et une vue
graphique temps réel (Win32).

## Structure

```
include/            en-têtes (.h)
  client/           AbstractClient, Client, VIPClient
                    AbstractOperation, Consultation, Transfer, Withdraw
  bank/             Bank, Cashier, Queue
  simulation/       SimulationEntry, SimulationUtility, StatisticManager, Simulation
  database/         Database (persistance SQLite)
  ui/               RealtimeView (fenêtre graphique Win32 / GDI)
src/                implémentations (.cpp), même arborescence
tests/              tests unitaires (un exécutable par fichier)
database/           database.sql — schéma de la base
third_party/sqlite/ SQLite (amalgamation officielle)
main.cpp            lance une simulation (console ou graphique) puis enregistre les résultats
build.ps1           script de compilation Windows / PowerShell
```

## Compilation

```powershell
.\build.ps1         # compile -> bank2.exe
.\build.ps1 -Run    # compile puis exécute en mode console
.\build.ps1 -Gui    # compile puis lance la fenêtre graphique temps réel
.\build.ps1 -Test   # compile et lance les tests unitaires
```

Ou directement : `.\bank2.exe` (console) / `.\bank2.exe gui` (fenêtre graphique).

Nécessite `g++` et `gcc` (C++17). Le script pointe vers le compilateur fourni
avec MSYS2 ; adapter la variable `$gccBin` dans `build.ps1` selon la machine.

## Base de données

À chaque exécution, `main` enregistre dans `bank2.db` (SQLite) :

- les **paramètres** de la simulation (table `simulations`) ;
- les **statistiques** calculées (table `statistics`) ;
- le **détail des clients** servis et non servis (tables `clients`, `operations`).

Puis il relit toutes les simulations enregistrées et les affiche sous forme de
tableau. Le fichier `bank2.db` est recréé s'il n'existe pas ; il n'est pas suivi
par git.

Schéma complet : [`database/database.sql`](database/database.sql).

## Vue graphique temps réel

`.\bank2.exe gui` ouvre une fenêtre (Win32 / GDI, sans dépendance externe) qui
déroule la simulation une unité de temps toutes les 150 ms : caissiers
libres (vert) / occupés (rouge), file d'attente avec clients normaux (bleu) et
VIP (or), et statistiques mises à jour en direct. À la fin, les résultats sont
enregistrés dans la base comme en mode console.
