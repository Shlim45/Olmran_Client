@echo off

set CommonCompilerFlags=-nologo -fp:fast -GR- -Od -Oi -WX -W4 -wd4100 -wd4005 -wd4189 -FC -Z7
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib ws2_32.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\code\win32_olmran.cpp /link %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\code\win32_olmran.cpp -Fmwin32_olmran.map /link %CommonLinkerFlags%

popd
