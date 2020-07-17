@echo off

REM
REM  To run this at startup, use this as your shortcut target:
REM  %windir%\system32\cmd.exe /k  C:\Users\Jon\Desktop\Programming\C\Projects\game_client\misc\shell.bat
REM

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set path=z:\misc;%path%
