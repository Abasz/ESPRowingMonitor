@echo off
echo [34m[4mRunning command[0m: mingw32-make e2e DEFINES+="-D LOG_CALIBRATION -D USE_CUSTOM_SETTINGS -D 'BOARD_PROFILE "profiles/generic.board-profile.h"' -D 'ROWER_PROFILE "profiles/generic.rower-profile.h"

mingw32-make --no-print-directory e2e DEFINES+="-D LOG_CALIBRATION -D USE_CUSTOM_SETTINGS -D 'BOARD_PROFILE=\""profiles/generic.board-profile.h\""' -D 'ROWER_PROFILE=\""profiles/generic.rower-profile.h\""'" || exit /b

setlocal enabledelayedexpansion

echo [34m[4mCurrent directory:[0m %~dp0

for %%f in ("%~dp0*test.txt") do (
  echo file: %%~nxf
  if not exist "%~dp0output" mkdir "%~dp0output"
  build\e2e\run_e2e_test.out "%%~ff" > "%~dp0output\%%~nxf-output.txt"
  
  for /F "tokens=*" %%g in ('type "%~dp0output\%%~nxf-output.txt" ^| find /i "power" /c') do (   
    set passed=FAILED
    set "color=[31m"

    for /F "tokens=3 delims=-" %%c in ('echo %%~nxf') do (
        if %%c == %%g set passed=PASSED & set "color=[32m"
    )
    echo number of strokes: %%g !color!!passed! [0m
  )
)
