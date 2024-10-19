@echo off
set astyle="D:\Project_All\duilib\Duilib_TaskbarTool\bin\astyle.exe"

REM 循环遍历 C/C++ 源文件
for /r . %%a in (*.cpp *.c) do (
    %astyle% --style=ansi --unpad-paren -s4 -n "%%a"
)

REM 循环遍历头文件
for /r . %%a in (*.hpp *.h) do (
    %astyle% --style=ansi --unpad-paren -s4 -n "%%a"
)

REM 删除所有的 astyle 生成文件
for /r . %%a in (*.orig) do (
    del "%%a"
)

pause