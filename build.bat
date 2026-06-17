@echo off
REM STM32F429BIT6 demo - build launcher (calls build.ps1)
REM
REM Usage:
REM   build.bat                  Debug build (default)
REM   build.bat -Config Release  Release build
REM   build.bat -Clean           Remove build dir only (no compile)
REM
REM Output: build\Debug\stm32f429bit6_demo.elf
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1" %*
exit /b %ERRORLEVEL%
