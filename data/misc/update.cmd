@echo off

set AppFileSlashesEsc=%~1
set InstallPath=%~2
set UpdatePath=%~3
set "PrevVersionBackupPath=%InstallPath%_old"

:: Wait for all processes running from this .exe to finish
:loop
wmic process get ExecutablePath | findstr /i "%AppFileSlashesEsc%" >nul
if %errorlevel% neq 1 (
    timeout /t 2 >nul
    goto :loop
)

:: Rename an application directory into a temporary folder with the previous version backup
move /y "%InstallPath%" "%PrevVersionBackupPath%"
if %errorlevel% neq 0 (
	set "ResultCode=10"
	set "ResultMsg=Failed to rename an installation directory to create a previous version backup"
    goto :run
)

:: Move the new version to the application directory
mkdir "%InstallPath%"
(robocopy "%UpdatePath%" "%InstallPath%" *.* /r:5 /w:2 /e /move >nul) & if %errorlevel% lss 8 set errorlevel=0
if %errorlevel% neq 0 (
    :: Restore backup
    move /y "%PrevVersionBackupPath%" "%InstallPath%"
	set "ResultCode=20"
	set "ResultMsg=Failed to move an update to the installation directory"
	goto :run
)

:: Remove a temporary folder with the previous version backup
rmdir /S /Q "%PrevVersionBackupPath%" | rem
if %errorlevel% neq 0 (
	set "ResultCode=30"
	set "ResultMsg=Updated successfully but failed to remove temporary folder %PrevVersionBackupPath%"
    goto :run
)

set "ResultCode=0"
set "ResultMsg=Updated successfully"

:run

:: First "" is an empty title for a new command prompt
start "" "%InstallPath%\ceed.exe" -updateResult %ResultCode% -updateMessage "%ResultMsg%"

