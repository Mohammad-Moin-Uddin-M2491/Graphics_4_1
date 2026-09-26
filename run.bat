@echo off
cd /d "%~dp0"
echo Compiling OpenGL Project...
g++ -std=c++17 -Wall -Wextra src/main.cpp src/glad.c -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 -o cog_demo.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build succeeded! Running cog_demo.exe...
    cog_demo.exe
) else (
    echo [ERROR] Build failed!
    pause
)
