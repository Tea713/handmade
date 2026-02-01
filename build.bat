@echo off

mkdir .\build
pushd .\build
cl -Zi ..\src\handmade.cpp gdi32.lib user32.lib
popd
