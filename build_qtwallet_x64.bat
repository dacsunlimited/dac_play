@echo off
REM call setenv_x64.bat 
if "%BITSHARES_ROOT%" == "" (
    echo please call setenv_x64.bat firstly
    goto done 
)

set WORKSPACE=%BITSHARES_ROOT%

REM ======= JENKINS_BUILD setttings ======= 
set BUILD_NUMBER="0"
set BUILD_URL="http://url"

REM =========================================================================
set MSVC_RUNTIME_DIR_X64=%BITSHARES_ROOT%\vc_runtime.x64

set PREBUILT_BINARY_DIR_SUFFIX=.x64
set BOOST_PREBUILT_BINARY_DIR_SUFFIX=.x64
set ARCHITECTURE_SHORT_NAME=x64
set BLANK_OR_SIXTY_FOUR=64
set VISUAL_STUDIO_CONFIGURATION_SUFFIX= Win64
set MSBUILD_PLATFORM_NAME=x64
set ICU_VERSION_TO_PACKAGE=53

set QT_PATH_X64=%BITSHARES_ROOT%\QT.x64
set OPENSSL_ROOT_DIR_X64=%BITSHARES_ROOT%\OpenSSL.x64
set CRASHRPT_BIN_DIR_X64=%BITSHARES_ROOT%\CrashRpt\bin\x64

set ENABLE_CRASHRPT_FLAG=FALSE

REM if exist %WORKSPACE%\bts_play_build (
REM  rd /S /Q %WORKSPACE%\bts_play_build
REM )
REM 
REM mkdir %WORKSPACE%\bts_play_build
REM 
REM cd %WORKSPACE%\bts_play_build
REM 

cmake %WORKSPACE%/bitshares_play ^
        -DBLOCKCHAIN_NAME:STRING=BitSharesPLAY ^
        -DINCLUDE_QT_WALLET:BOOL=TRUE ^
        -DCMAKE_PREFIX_PATH=%QTDIR% ^
        -DICU_VERSION_TO_PACKAGE:STRING=%ICU_VERSION_TO_PACKAGE% ^
        -DINCLUDE_CRASHRPT:BOOL=%ENABLE_CRASHRPT_FLAG% ^
        -DTARGET_ARCHITECTURE:STRING=%ARCHITECTURE_SHORT_NAME% ^
        -DMSVC_RUNTIME_DIR:PATH=%MSVC_RUNTIME_DIR_X64% ^
        -DGET_VERSION_STRING_FROM_GIT_TAGS:BOOL=TRUE ^
        -G "Visual Studio 12%VISUAL_STUDIO_CONFIGURATION_SUFFIX%" ^
        -T "v120_xp" ^
        -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
        -DBTS_CLIENT_JENKINS_BUILD_NUMBER:STRING=%BUILD_NUMBER% ^
        -DBTS_CLIENT_JENKINS_BUILD_URL:STRING="%BUILD_URL%" 

if %errorlevel% neq 0 exit /b %errorlevel%

echo msbuild.exe /M:4 /p:Configuration=RelWithDebInfo /p:Platform=%MSBUILD_PLATFORM_NAME% BitShares.sln
if %errorlevel% neq 0 exit /b %errorlevel%

rem "C:\Program Files (x86)\Inno Setup 5\iscc.exe" "%WORKSPACE%\build\programs\qt_wallet\setup.iss"
rem if %errorlevel% neq 0 exit /b %errorlevel%

rem cd %WORKSPACE%\build\programs\qt_wallet
rem %SHA1SUM_EXE% BitShares-*.exe > sha1sum.txt

rem cd %WORKSPACE%\build
rem "%SEVENZIP_EXE%" a pdbs.7z -ir!*.pdb


:done

