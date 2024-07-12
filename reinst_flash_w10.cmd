@echo off
rem
rem Are we even needed?
rem
wmic qfe get hotfixid | find /i "KB4577586" >nul
if errorlevel 0 echo [*] Evil update KB4577586 installed.
if not exist %systemroot%\System32\Macromed\Flash\Flash.ocx goto noflash
if exist %systemroot%\SysWOW64 (
  if not exist %systemroot%\SysWOW64\Macromed\Flash\Flash.ocx goto noflash
)
if "%1"=="/FORCE" goto noflash
echo Flash is still present, are you sure that you want me to run?
echo If so, start me with /FORCE parameter
goto fini

:noflash
rem Ensure that we are run from 64bit prompt
if "%ProgramFiles(x86)%" == "" goto StartExec
if not exist %SystemRoot%\Sysnative\cmd.exe goto StartExec
%SystemRoot%\Sysnative\cmd.exe /C "%~f0" %*
exit /b
:StartExec
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
if '%errorlevel%' NEQ '0' (
  echo Requesting administrative privileges...
  goto UACPrompt
) else ( goto gotAdmin )
:UACPrompt
echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
echo UAC.ShellExecute "%~s0", "%1", "%1", "runas", 1 >> "%temp%\getadmin.vbs"
"%temp%\getadmin.vbs"
exit /B
:gotAdmin
if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
pushd "%CD%"
CD /D "%~dp0"

setlocal
set ARCH=amd64
reg query HKLM\Hardware\Description\System\CentralProcessor\0 /v Identifier | Find /i "x86" >nul
if not errorlevel 1 set ARCH=wow64

if not exist KB4580325.msu (
  echo [*] Downloading update...
  bitsadmin /transfer KB4580325.msu /download /priority normal http://download.windowsupdate.com/d/msdownload/update/software/secu/2020/10/windows10.0-kb4580325-x64_b6e8f5b34fd68a4e3c29a540000327f6d0675a7f.msu %CD%\KB4580325.msu
  if not exist KB4580325.msu (
    echo [-] Download failed
    goto fini
  )
)

echo [*] Extracting contents...
md ContentMSU
expand KB4580325.msu /F:* .\ContentMSU >nul
cd ContentMSU
md ContentCAB
expand Windows*.cab /F:* .\ContentCAB >nul
cd ContentCAB

echo [*] Copying Flash..
cd %ARCH%_adobe*
call :copyflash System32
if "%ARCH%"=="amd64" (
  cd ..\wow64_adobe*
  call :copyflash SysWOW64
  copy /y flashplayercplapp.cpl %systemroot%\SysWOW64\
  copy /y flashplayerapp.exe %systemroot%\SysWOW64\
  set ARCH=x64
) else (
  copy /y flashplayercplapp.cpl %systemroot%\System32\
  copy /y flashplayerapp.exe %systemroot%\System32\
  set ARCH=x86
)
cd ..\..\..
rmdir /s /q  ContentMSU
if not exist flashpatch_%ARCH%.exe (
  echo [-] flashpatch_%ARCH%.exe not found!
  goto fini
)
flashpatch_%ARCH%.exe
exit /B

:copyflash
md %systemroot%\%1\Macromed\Flash
md %systemroot%\%1\Macromed\Temp
copy /y flash.ocx %systemroot%\%1\Macromed\Flash\
copy /y flashutil_activex.dll %systemroot%\%1\Macromed\Flash\
copy /y flashutil_activex.exe %systemroot%\%1\Macromed\Flash\
copy /y activex.vch %systemroot%\%1\Macromed\Flash\
regsvr32 /s %systemroot%\%1\Macromed\Flash\flash.ocx
exit /B

:fini
pause
