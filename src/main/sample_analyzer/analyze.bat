
@Echo OFF

rem Quotes around entire argument of 'set' prevent problems when the path or a directory name contains '&'
rem (which is legal in Windows and occurs with some MySQL setups)

set "BASEDIR=%~dp0"

set "OLDPATH=%PATH%"
set "PATH=%PATH%;%BASEDIR%\..\..\dependencies\boost\lib;%BASEDIR%\..\..\dependencies\zlib\bin;%BASEDIR%\..\..\dependencies\icu\bin64;%BASEDIR%\..\lib;%BASEDIR%\..\bin;

set "FREELINGSHARE=%BASEDIR%..\share\freeling"

set cmdline=

:loop
if "%~1" EQU "" goto :endloop

  set cmdline=%cmdline% %~1

  if "%~1" NEQ "-f" goto :endif
     SHIFT 
     if exist %~1 goto :else
       set cmdline=%cmdline% %FREELINGSHARE%\config\%~1  
       goto :endif
     :else       
       set cmdline=%cmdline% %~1
  :endif

  SHIFT

  goto :loop
:endloop

analyzer.exe %cmdline%

SET "PATH=%OLDPATH%"
SET BASEDIR=
SET FREELINGSHARE=
SET cmdline=
