@echo off
setlocal enableextensions enabledelayedexpansion

rem === Clean and (re)create build dir ===

mkdir build

cd lib

if not exist vcpkg (
git clone https://github.com/microsoft/vcpkg.git 
)

cd..

set "curDir=%~dp0"
pushd "%curDir%build" ||	 (
  echo Failed to enter build directory.
  exit /b 1
)	

rem ---- Configure (Visual Studio 2022, x64) ----
cmake -G "Visual Studio 17 2022" -A x64 .. 
if errorlevel 1 (
  echo CMake configure failed.
  popd
  pause
)
popd

cmake --build build --target Scripts --config Release
cmake --build build --target Scripts --config Debug
cmake --build build --target Scripts --config Package


echo Builds completed successfully.

pause
