@echo off
setlocal
set "dirname=%~dp0"
if %0 == "%~dpnx0" where /q "%cd%:%~nx0" && set "dirname=%cd%\"
if exist "%dirname%Editor.exe" (set "DEBUG=") else (set "DEBUG=_d")
if [%1] == [] (set "OPT1=-w -s") else (set "OPT1=")
start "" "%dirname%Editor%DEBUG%" Scripts/Editor.as %OPT1% %* -showconsole
