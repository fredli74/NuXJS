@ECHO OFF

REM Install timeit.exe from Windows 2003 Resource Kit

SET EXE=Release\NuXJScript -s -t

FOR %%f IN (perftest*.js) DO (
	ECHO %%f
	ECHO.
	timeit -s %EXE% %%f
)
