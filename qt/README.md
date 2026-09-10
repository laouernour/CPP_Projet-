# Interface Qt — Simulation Banque

Une fenêtre, un bouton Démarrer/Pause/Reprendre, trois guichets et quatre compteurs.
Les personnages, portes, guichets, étoiles et barres sont dessinés avec QPainter.
Une ligne complémentaire affiche l'attente moyenne, l'occupation et la satisfaction
calculées par StatisticManager. Les clients en attente portent les indications
« Arrivée : X u.t. », « Patience : Y u.t. », leur opération et, si nécessaire, « VIP ».
L'approche est signalée par « EN APPROCHE », sans durée. Au guichet, le temps réel
restant est affiché sous la forme « Reste X u.t. ».

## Lancer la version compilée

Depuis `C:\Users\HP\Desktop\c++\bank2_cpp\qt` :

```powershell
.\release\bank_sim_gui.exe
```

## Recompiler sous PowerShell

Fermer l'exécutable avant de le recompiler.

```powershell
Set-Location 'C:\Users\HP\Desktop\c++\bank2_cpp\qt'
$env:Path = 'C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;' + $env:Path

# Recrée les Makefiles pour le compilateur Qt installé.
qmake .\bank_sim_gui.pro -spec win32-g++
if ($LASTEXITCODE -ne 0) { throw 'Échec de qmake' }

# Facultatif : recompilation complète, sans toucher aux sources ni aux bases.
mingw32-make clean
if ($LASTEXITCODE -ne 0) { throw 'Échec du nettoyage' }

mingw32-make -j4
if ($LASTEXITCODE -ne 0) { throw 'Échec de compilation' }

# Copie les bibliothèques nécessaires près de l'exécutable.
windeployqt --release --no-translations .\release\bank_sim_gui.exe
if ($LASTEXITCODE -ne 0) { throw 'Échec du déploiement des DLL' }

.\release\bank_sim_gui.exe
```

## Graphiques statistiques

L'onglet **Statistiques** présente des barres, un camembert et deux courbes
en escalier des départs cumulés (servis et abandons). Les axes indiquent les
nombres de clients et le temps en unités de simulation. Les clients encore
en file ou au guichet sont exclus de la répartition des départs.

La liste permet de choisir la simulation en cours ou une simulation enregistrée.
L'historique est relu dans `bank2.db`, à côté de l'exécutable, à chaque ouverture
de l'onglet et après une sauvegarde. Les courbes historiques sont reconstruites
à partir des heures de départ de la table `clients`, puis vérifiées contre les
totaux de `statistics`. Une erreur de lecture apparaît dans l'onglet.
Le choix d'un historique ne suspend pas la simulation en cours ; les compteurs
du bas restent ceux de cette simulation. Le bouton Pause permet de la suspendre.
Les graphiques utilisent QPainter sans module Qt supplémentaire.

## Synchronisation

- `SimulationDefaults.h` fournit les mêmes paramètres à la console et à Qt.
  La durée, les guichets, les services, les arrivées, le taux VIP et la patience
  sont lus dans SimulationEntry ; changer ces paramètres ne nécessite pas
  de modifier BankScene ni les attentes chiffrées des tests d'affichage.
- `MainWindow` possède la vraie `Simulation`. Son `simulationTimer` tente
  un pas toutes les 650 ms. Le premier pas est exécuté dès le clic.
- Le timer avance le moteur sans attendre les entrées, déplacements de file
  ou approches des guichets. La progression des personnages et la fréquence
  de rendu ne conditionnent jamais un appel à `Simulation::step()`.
- `BankScene` possède uniquement des représentations graphiques, indexées
  par les adresses des vrais clients. Les décisions viennent de la file,
  des caissiers et des listes de départs du moteur.
- Un client affecté immédiatement entre tout de même par la porte, traverse
  l'accueil et rejoint son guichet. Il n'est pas ajouté à une fausse file.
- Le guichet indique « Occupé / Client en approche » dès l'affectation réelle.
  Son compteur reste celui du moteur, même pendant le trajet graphique.
  Si le moteur décide un départ avant la fin du trajet, la scène adopte cet
  état et anime la sortie sans inventer un service supplémentaire.
- L'`animationTimer` tourne toutes les 16 ms. La distance parcourue utilise
  le temps écoulé et une vitesse de 285 pixels logiques par seconde ; le
  déplacement est borné par la distance restante, y compris aux étapes du trajet.
- La patience est interpolée vers la dernière valeur du moteur. À zéro,
  elle est affichée immédiatement à zéro et le personnage devient rouge.
- La file s'étend en rangées serpentines de huit places uniques. Le contenu
  est ajusté automatiquement en largeur et en hauteur pour rester entièrement
  visible sans défilement. Une longue file réduit proportionnellement le dessin.
  Les rangées réservent la hauteur nécessaire aux informations des clients.
- Pause fige le moteur et les personnages. L'animation graphique ne fait
  jamais avancer `Simulation`.
- Le temps affiché est le dernier instant traité : `getCurrentTime() - 1`,
  borné entre zéro et la durée. Le libellé est « Temps : X / Y u.t. » et vaut
  exactement la durée configurée en fin de simulation. La convention du moteur reste `0..durée`.
- À la fin, seul le timer du moteur s'arrête. Les trajets déjà décidés finissent.
  Les clients encore en file ou en service restent visibles ; aucun départ
  supplémentaire n'est inventé pour vider la banque.

Les VIP et les opérations restent aléatoires : chaque lancement peut montrer
une répartition différente, exactement comme la console.

## SQLite

Les résultats sont enregistrés une seule fois, après la fin du moteur, via
la classe `Database` existante. Le fichier Qt est `bank2.db` à côté de
l'exécutable (`qt/release/bank2.db`), quel que soit le dossier de lancement.
Le message de réussite apparaît seulement après validation de la sauvegarde.
Une erreur apparaît discrètement en bas ; son détail est dans l'infobulle.
Le schéma et les statistiques du moteur ne sont pas modifiés par la refonte.

## Vérifications Qt

Les tests nécessitent le module Qt Test fourni par l'installation Qt.
Ils utilisent le vrai moteur pour les parcours complets et ne lancent
aucune fenêtre visible avec la plateforme `offscreen`.

```powershell
Set-Location 'C:\Users\HP\Desktop\c++\bank2_cpp'
$env:Path = 'C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;' + $env:Path
New-Item -ItemType Directory -Force .\build\qt-tests | Out-Null
Set-Location .\build\qt-tests
qmake ..\..\qt\tests\bank_scene_test.pro -spec win32-g++
mingw32-make -j4
.\release\bank_scene_test.exe -platform offscreen
```

Les captures `bank-ready.png`, `bank-running.png` et `bank-finished.png` sont
créées dans le dossier d'exécution des tests. La sauvegarde de test reste
isolée dans `build/qt-tests/release/bank2.db`.
