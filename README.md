# bank2_cpp

Simulation d'une banque en C++17 : opérations bancaires polymorphes, clients
(standard / VIP), file d'attente avec priorité et patience, et caissiers.

## Structure

```
include/        en-têtes (.h)
  client/       AbstractClient, Client, VIPClient
                AbstractOperation, Consultation, Transfer, Withdraw
  bank/         Bank, Cashier, Queue
src/            implémentations (.cpp), même arborescence
main.cpp        test de fumée (démo d'intégration)
build.ps1       script de compilation Windows / PowerShell
```

## Compilation

```powershell
.\build.ps1        # compile -> bank2.exe
.\build.ps1 -Run   # compile puis exécute
```

Nécessite `g++` (C++17). Le script pointe vers le g++ fourni avec MSYS2 ;
adapter la variable `$gccBin` dans `build.ps1` selon la machine.

Compilation manuelle équivalente :

```
g++ -std=c++17 -Wall -Wextra -Iinclude main.cpp src/**/*.cpp -o bank2.exe
```
