@ECHO OFF
set VCDIR=c:\Program Files\Microsoft Platform SDK
set MSSDK=c:\Program Files\Microsoft Platform SDK
set CLFLAGS=/Wall /WX /wd4100 /wd4201 /wd4204 /wd4255 /wd4310 /wd4619 /wd4668 /wd4701 /wd4706 /wd4711 /wd4820 /wd4826
set BIT=64

::### Set paths ###::
if "%BIT%" == "32" (
  set "PATH=%MSSDK%\bin;%VCDIR%\bin;%VCDIR%\..\Common7\IDE;%VCDIR%\..\Common\MSDev98\Bin;%PATH%"
  set "INCLUDE=%MSSDK%\include;%MSSDK%\include\crt;%VCDIR%\include;%INCLUDE%"
  set "LIB=%MSSDK%\lib;%VCDIR%\lib;%LIB%"
  set MACHINE=I386
) else (
  set "PATH=%MSSDK%\bin\win64\x86\amd64;%MSSDK%\bin;%VCDIR%\bin\x86_amd64;%VCDIR%\..\Common7\IDE;%PATH%"
  set "INCLUDE=%MSSDK%\include;%MSSDK%\include\crt;%VCDIR%\include;%INCLUDE%"
  set "LIB=%MSSDK%\lib\amd64;%MSSDK%\lib\x64;%VCDIR%\lib\amd64;%LIB%"
  set MACHINE=AMD64
)

::### Compile ###::
if "%VCDIR%" == "%VCDIR:2003=%" (
  if "%VCDIR%" == "%VCDIR:VC98=%" set CLFLAGS=%CLFLAGS% /GS-
)
rc /R /DRC_VERSIONBIT=%BIT% /Fo"Stats.res" "Resources\Stats.rc"
if not %ERRORLEVEL% == 0 set EXITCODE=%ERRORLEVEL%
cl /O1 %CLFLAGS% Stats.c Stats.res /LD /link kernel32.lib user32.lib /OPT:NOWIN98 /MACHINE:%MACHINE% /NODEFAULTLIB /ENTRY:DllMain
if not %ERRORLEVEL% == 0 set EXITCODE=%ERRORLEVEL%

::### Clean up ###::
if exist Stats.res del Stats.res
if exist Stats.obj del Stats.obj
if exist Stats.lib del Stats.lib
if exist Stats.exp del Stats.exp

::### End ###::
if not "%1" == "/S" @PAUSE
if defined EXITCODE exit %2 %EXITCODE%
