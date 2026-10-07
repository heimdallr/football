rem @echo off

set start_time=%DATE% %TIME%

call src\ext\scripts\batch\check_executable.bat cmake
if NOT [%ERRORLEVEL%]==[0] goto end

set tee_name=tee.exe
call src\ext\scripts\batch\check_executable.bat %tee_name%
if NOT [%ERRORLEVEL%]==[0] goto end

if [%BUILD_TYPE%]==[] set BUILD_TYPE=Debug
set BUILD_DIR=%~dp0build\%BUILD_TYPE%
mkdir %BUILD_DIR%
del %BUILD_DIR%\*.sln

set QT_MAJOR_VERSION=6
set PLATFORM=x86_64
set QT_DIR=D:/sdk/Qt/Qt6/6.12.0/msvc2022_%PLATFORM%_%BUILD_TYPE%/lib/cmake/Qt6

cmake -B %BUILD_DIR% ^
--no-warn-unused-cli ^
-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
-DPLATFORM=%PLATFORM% ^
-DQT_MAJOR_VERSION=%QT_MAJOR_VERSION% ^
-DQt%QT_MAJOR_VERSION%_DIR=%QT_DIR% ^
-D7zip_BIN_DIR=D:/sdk/7z/x64/bin ^
-DCPACK_GENERATOR=WIX ^
%* ^
-G "Visual Studio 17 2022" %~dp0 2>&1 | %tee_name% %BUILD_DIR%\configure.log

echo -- Start: %start_time%
echo -- Stop:  %DATE% %TIME%

:end
