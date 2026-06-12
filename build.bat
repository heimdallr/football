@echo off

set BUILD_TYPE=Release
set BUILD_DIR=%~dp0build\%BUILD_TYPE%

del /s /q %BUILD_DIR%\bin\*
del /s /q %BUILD_DIR%\football\*

call %~dp0configure.bat %*
if %errorlevel% NEQ 0 goto Error

set start_time=%DATE% %TIME%

echo building
cmake --build %BUILD_DIR% --config Release
if %errorlevel% NEQ 0 goto Error

cmake --install %BUILD_DIR% --prefix %BUILD_DIR%/football

rem echo testing
rem ctest --test-dir %BUILD_DIR% -C Release
rem if %errorlevel% NEQ 0 goto Error

goto End

:Error
echo someting went wrong :(
exit /B 1

:End
echo working time
echo -- Start: %start_time%
echo -- Stop:  %DATE% %TIME%
