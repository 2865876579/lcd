@echo off
setlocal EnableExtensions

cd /d "%~dp0"
call "%~dp0build_flash.bat" COM3
