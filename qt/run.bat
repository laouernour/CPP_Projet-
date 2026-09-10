@echo off
rem Lance la vue Qt sans rien changer au PATH du systeme.
rem Double-clic sur ce fichier, ou :  .\run.bat  depuis le dossier qt\
cd /d "%~dp0release"
start "" "bank_sim_gui.exe"
