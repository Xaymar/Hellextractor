@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET "SCRIPT=%~0"
SET "SCRIPTROOT=%~dp0"

:FILELOOP
	CALL :INDEX "%~1"
	SHIFT /1
	IF "%~n1" == "" (
		GOTO :EOF
	)
GOTO :FILELOOP

:CHECK
	IF "%~nx1"=="$RECYCLE.BIN" (
		ECHO. > NUL
	) ELSE IF "%~nx1"=="System Volume Information" (
		ECHO. > NUL
	) ELSE IF "%~nx1"==".." (
		ECHO. > NUL
	) ELSE IF "%~nx1"=="." (
		ECHO. > NUL
	) ELSE IF "%~nx1"=="" (
		ECHO. > NUL
	) ELSE IF NOT "%~x1"=="" (
		ECHO. > NUL
	) ELSE (
		CALL :INDEX "%~1"
	)
EXIT /B 0

:INDEX
	SET "IS_DIR=0"
	ECHO.%~a1 | find "d" >NUL 2>NUL && (
		set "IS_DIR=1"
	)
	if "!IS_DIR!"=="1" (
		FOR %%i IN ("%~1\*") DO (
			CALL :CHECK "%%~i"
		)
		FOR /D %%i IN ("%~1\*") DO (
			CALL :CHECK "%%~i"
		)
	) ELSE (
		CALL :TRANSCODE "%~1"
	)
EXIT /B 0

:TRANSCODE
	ECHO %~dpn1
	"%SCRIPTROOT%\quickbms.exe" -Q -o -. -e "%SCRIPTROOT%\datafile.bms" "%~1" "%SCRIPTROOT%\output"
	if ERRORLEVEL 1 (
		PAUSE
	)
EXIT /B 0
