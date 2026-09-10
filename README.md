# bank2_cpp

Simulation d'une banque en C++17 : opérations bancaires polymorphes, clients
(standard / VIP), file d'attente avec priorité et patience, caissiers,
enregistrement des résultats dans une base de données SQLite, et une vue
graphique temps réel avec Qt Widgets.

## Structure

```
include/            en-têtes (.h)
  client/           AbstractClient, Client, VIPClient
                    AbstractOperation, Consultation, Transfer, Withdraw
  bank/             Bank, Cashier, Queue
  simulation/       SimulationEntry, SimulationUtility, StatisticManager, Simulation
  database/         Database (persistance SQLite)
src/                implémentations (.cpp), même arborescence
tests/              tests unitaires (un exécutable par fichier)
database/           database.sql — schéma de la base
include/database/sqlite/ en-tete SQLite
src/database/sqlite/ implementation SQLite
qt/                 interface graphique Qt et tests graphiques
main.cpp            lance la console puis enregistre les résultats
```

Le projet contient les sources de l'application console et de l'interface Qt.
La version console déjà compilée peut être lancée directement avec
`.\bank2.exe`. Pour l'animation, lancer `.\qt\release\bank_sim_gui.exe`.

Les paramètres communs sont dans `include/simulation/SimulationDefaults.h` :
100 unités, 3 caissiers, services de 10 à 20 unités, une arrivée toutes les
2 unités, 10 % de VIP et une patience de 5 unités.

## Compilation console (PowerShell)

Depuis la racine du projet, avec le compilateur fourni par Qt :

```powershell
$env:Path = 'C:\Qt\Tools\mingw1310_64\bin;' + $env:Path
New-Item -ItemType Directory -Force .\build\console | Out-Null
$sources = @(Get-ChildItem .\src -Recurse -Filter *.cpp | ForEach-Object FullName)
gcc -O2 -c .\src\database\sqlite\sqlite3.c -o .\build\console\sqlite3.o
if ($LASTEXITCODE -ne 0) { throw 'Compilation SQLite impossible' }
g++ -std=c++17 -Wall -Wextra -Iinclude .\main.cpp $sources .\build\console\sqlite3.o -o .\bank2.exe
if ($LASTEXITCODE -ne 0) { throw 'Compilation console impossible' }
.\bank2.exe
```

Pour compiler et lancer les tests métier, après la compilation de SQLite :

```powershell
New-Item -ItemType Directory -Force .\build\tests | Out-Null
foreach ($test in Get-ChildItem .\tests -Filter *.cpp) {
    $exe = Join-Path $PWD "build\tests\$($test.BaseName).exe"
    g++ -std=c++17 -Wall -Wextra -Iinclude $test.FullName $sources .\build\console\sqlite3.o -o $exe
    if ($LASTEXITCODE -ne 0) { throw "Compilation échouée : $($test.Name)" }
    & $exe
    if ($LASTEXITCODE -ne 0) { throw "Test échoué : $($test.Name)" }
}
```

## Base de données

À chaque exécution, `main` enregistre dans `bank2.db` (SQLite) :

- les **paramètres** de la simulation (table `simulations`) ;
- les **statistiques** calculées (table `statistics`) ;
- le **détail des clients** servis et non servis (tables `clients`, `operations`).

Puis il relit toutes les simulations enregistrées et les affiche sous forme de
tableau. Le fichier `bank2.db` est recréé s'il n'existe pas ; il n'est pas suivi
par git.

Schéma complet : [`database/database.sql`](database/database.sql).

## Interface graphique Qt

### Version Qt Widgets (`qt/`)

Une agence animée avec QPainter : un seul bouton Démarrer/Pause/Reprendre,
une porte d'entrée, une file commune, trois guichets et une sortie distincte.
Les quatre compteurs affichent les arrivées, l'attente, les services terminés
et les abandons de la vraie simulation C++.

Les paramètres sont partagés avec la console via `SimulationDefaults.h`.
Un timer pilote `Simulation::step()` et un second anime les déplacements à
vitesse constante (~60 FPS), sans dépasser les destinations. Les longues
files occupent plusieurs rangées distinctes dans une scène défilable.

Les clients VIP portent une étoile dorée. L'opération et la patience viennent
du moteur ; à patience zéro, les personnages deviennent rouges. Les opérations
urgentes restent en file, les abandons et les départs sont animés vers la sortie.

À la fin, un message discret confirme la sauvegarde via `Database` dans
`qt/release/bank2.db`. Les clients encore présents restent affichés conformément
à l'état final du moteur.

Les commandes PowerShell complètes, les conventions d'animation et les tests
Qt sont décrits dans [`qt/README.md`](qt/README.md).
