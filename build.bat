@echo off

mkdir .\build
pushd .\build
cl -FC -Zi ..\src\handmade.cpp gdi32.lib user32.lib
popd
