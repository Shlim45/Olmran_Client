@echo off

set Wildcard=*.h *.cpp *.inl *.c

echo ----------
echo ----------

echo NOTES FOUND:
findstr -s -n -i -l "NOTE(jon)" %Wildcard%

echo ----------
echo ----------

echo TODOS FOUND:
findstr -s -n -i -l "TODO" %Wildcard%

echo ----------
echo ----------
