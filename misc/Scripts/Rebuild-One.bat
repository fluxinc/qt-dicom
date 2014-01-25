:: This batch file is used by the Build-All PowerShell script and probably
:: shouldn't be used on its own.
::
:: It tries to build the QtDicom solution (passed as the first parameter) using
:: compiler and settings for given architecture and configuration.
:: If it succeedes returned value is 0. Otherwise something different than that.
::
::
:: Example: Rebuild-One.bat "..\..\Qt Flux.sln" x86 Debug
::          Rebuild-One.bat "..\..\Qt Flux.sln" x64 Release
::
::
:: Author: Pawel Zak <pawel.zak@fluxinc.ca>

@ECHO ON

@IF NOT [%1] EQU [] (
	@SET SOLUTION=%1
) ELSE (
	@ECHO Missing path to the solution file
	@EXIT /B 1
)


@IF NOT [%2] EQU [] (
	@SET ARCH=%2
) ELSE (
	@ECHO Missing architecture label
	@EXIT /B 1
)

@IF NOT [%3] EQU [] (
	@SET CONF=%3
) ELSE (
	@ECHO Missing configuration label
	@EXIT /B 1
)
	

:: Check if Qt Dir base is set...
@IF "%QTDIRBASE%" EQU "" (
	@ECHO Unspecified Qt Dir base directory
	@EXIT /B 1
)

:: ...if it is, add architecture-dependant suffixes to create full path.
@IF %ARCH%==x86 (
	SET QTDIR=%QTDIRBASE%\x86
) ELSE (
	SET QTDIR=%QTDIRBASE%\amd64
)

:: Resolve path to SetEnv command file, used to switch between different versions
:: of compilers and configurations. SetEnv.cmd also makes MSBUILD accessible due
:: to %PATH% modifications
@IF NOT "%SDKDIR%" EQU "" (
	@SET SETENV="%SDKDIR%\Bin\SetEnv.cmd"
) ELSE (
	@ECHO Unspecified Windows SDK directory
	@EXIT /B 1
)

:: Call SetEnv.cmd. That should prepare our environment for the build process.
@CALL %SETENV% /%ARCH% /%CONF% 2>&1

:: While SetEnv.cmd requires /x86 switch to use 32-bit platform, Visual Studio
:: uses Platform = Win32. Talk about conventions...
@IF %ARCH%==x86 (
	SET ARCH=Win32
)

:: Execute MSBUILD tool and return from this script with its exit code
@MSBUILD /m /t:Clean,Build "/p:Configuration=%CONF%;Platform=%ARCH%;QtDir=%QTDIR%" %SOLUTION%
@EXIT /B %ERRORLEVEL%
