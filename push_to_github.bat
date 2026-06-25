@echo off
setlocal EnableExtensions EnableDelayedExpansion

cd /d "%~dp0"

set "REMOTE_URL=https://github.com/2865876579/lcd.git"
set "BRANCH=main"

where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: git was not found in PATH.
    echo Install Git for Windows first: https://git-scm.com/download/win
    pause
    exit /b 1
)

if not exist ".git" (
    echo [1/5] Initializing git repository...
    git init
    if errorlevel 1 goto fail
) else (
    echo [1/5] Git repository already exists.
)

echo [2/5] Configuring remote origin...
git remote get-url origin >nul 2>nul
if errorlevel 1 (
    git remote add origin "%REMOTE_URL%"
) else (
    git remote set-url origin "%REMOTE_URL%"
)
if errorlevel 1 goto fail

echo [3/5] Switching to %BRANCH% branch...
git checkout -B "%BRANCH%"
if errorlevel 1 goto fail

echo [4/5] Staging and committing changes...
git add -A
if errorlevel 1 goto fail

git diff --cached --quiet
if errorlevel 1 (
    set "COMMIT_MSG=Update LCD project %date% %time%"
    git commit -m "!COMMIT_MSG!"
    if errorlevel 1 goto commit_fail
) else (
    echo No local changes to commit.
)

echo [5/5] Pushing to GitHub...
git push -u origin "%BRANCH%"
if errorlevel 1 goto push_fail

echo.
echo Done: pushed to %REMOTE_URL%
pause
exit /b 0

:commit_fail
echo.
echo COMMIT FAILED.
echo If this is your first Git commit on this computer, run:
echo   git config --global user.name "Your Name"
echo   git config --global user.email "you@example.com"
pause
exit /b 1

:push_fail
echo.
echo PUSH FAILED.
echo Check that you are logged into GitHub in Git Credential Manager,
echo and that you have permission to push to:
echo   %REMOTE_URL%
echo.
echo If the remote repository already has unrelated commits, pull or clean it first.
pause
exit /b 1

:fail
echo.
echo FAILED. See the git error above.
pause
exit /b 1

