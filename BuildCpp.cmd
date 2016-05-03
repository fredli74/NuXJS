@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

SET options=/Ox /GL /MT /GS- /GR- /D NDEBUG
SET model=-m32

SET pfpath=%ProgramFiles(x86)%

IF NOT DEFINED pfpath SET pfpath=%ProgramFiles%

IF NOT DEFINED VCINSTALLDIR (
	IF EXIST "%pfpath%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" (
		CALL "%pfpath%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86 >NUL
	) ELSE (
		IF EXIST "%pfpath%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" (
			CALL "%pfpath%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" >NUL
		) ELSE (
			ECHO Could not find Visual C++ in one of the standard paths.
			ECHO Manually run vcvarsall.bat first or run this batch from a Visual Studio command line.
			EXIT /B 1
		)
	)
)
	

IF "%~1"=="debug" (
	SET options=/Od /MTd /GS /GR /Zi /D DEBUG
	SHIFT
) ELSE IF "%~1"=="beta" (
	SET options=/Ox /MTd /GS /GR /Zi /D DEBUG
	SHIFT
) ELSE IF "%~1"=="release" (
	SET options=/Ox /GL /MT /GS- /GR- /D NDEBUG
	SHIFT
)

IF "%~1"=="64" (
	ECHO Not yet implemented
	EXIT /B 1
	SHIFT
) ELSE IF "%~1"=="32" (
	SHIFT
)

SET args=%1
SET name=%~1
SHIFT

IF "%name%"=="" (
	ECHO BuildCpp [debug^|beta^|release] [32^|64 ^(bit^)] ^<output.exe^> ^<compiler options and source files^>
	EXIT /B 1
)

:argLoop
	IF "%~1"=="" goto argLoopEnd
	SET args=%args% %1
	SHIFT
goto argLoop
:argLoopEnd

SET temppath=%TEMP:"=%\%name%\

MKDIR "%temppath%" >NUL
ECHO Compiling %name%...
cl /nologo /W3 /WX- /GF /Gm- /EHsc /Gy %options% /arch:SSE /fp:precise /Zc:wchar_t /Zc:forScope /Gd /wd"4355" /wd"4267" /analyze- /errorReport:queue /D "WIN32" /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_SCL_SECURE_NO_WARNINGS" /Fo"%TEMP:"=%\\%name%\\" /Fe%args% >"%temppath%\buildlog.txt"
IF ERRORLEVEL 1 (
	TYPE "%temppath%\buildlog.txt"
	ECHO Compilation of %name% failed
	DEL /Q "%temppath%\*" >NUL
	RMDIR /Q "%temppath%" >NUL
	EXIT /B 1
) ELSE (
	ECHO Compiled %name%
	DEL /Q "%temppath%\*" >NUL
	RMDIR /Q "%temppath%\" >NUL

	EXIT /B 0
)
