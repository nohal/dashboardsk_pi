::
:: Install build dependencies. Requires a working choco installation,
:: see https://docs.chocolatey.org/en-us/choco/setup.
::
:: Initial run will do choco installs requiring administrative
:: privileges.
::

:: Install choco cmake and add it to the current session PATH
::
set CMAKE_HOME=C:\Program Files\CMake
if not exist "%CMAKE_HOME%\bin\cmake.exe" choco install --no-progress -y cmake
set "PATH=%PATH%;%CMAKE_HOME%\bin"

:: Install choco poedit and add it to the current session PATH
::
set POEDIT_HOME=C:\Program Files\Poedit\Gettexttools
if not exist "%POEDIT_HOME%" choco install --no-progress -y poedit
set "PATH=%PATH%;%POEDIT_HOME%\bin"

:: Update required python stuff
::
python --version > nul 2>&1 && python -m ensurepip > nul 2>&1
if errorlevel 1 choco install --no-progress -y python
python --version
python -m ensurepip
python -m pip install --upgrade pip
python -m pip install -q setuptools wheel
python -m pip install -q cloudsmith-cli
python -m pip install -q cryptography

:: Install pre-compiled wxWidgets and other DLL; add required paths.
::
set SCRIPTDIR=%~dp0
set "WX_VERSION=3.2.2.1"
set "WX_MSW_VERSION=3.2.2"
set "WX_DIR=wxWidgets"
set "WX_ARCH="
set "WX_LIB_DIR=vc_dll"
if /I "%PLATFORM%"=="x64" (
  set "WX_VERSION=3.2.9"
  set "WX_MSW_VERSION=3.2.9"
  set "WX_DIR=wxWidgets-x64"
  set "WX_ARCH=_x64"
  set "WX_LIB_DIR=vc14x_x64_dll"
)
set WXWIN=%SCRIPTDIR%..\cache\%WX_DIR%
set wxWidgets_ROOT_DIR=%WXWIN%
set wxWidgets_LIB_DIR=%WXWIN%\lib\%WX_LIB_DIR%
if not exist "%WXWIN%" (
  wget --version > nul 2>&1 || choco install --no-progress -y wget
  wget https://github.com/wxWidgets/wxWidgets/releases/download/v%WX_VERSION%/wxWidgets-%WX_VERSION%-headers.7z ^
      -O wxWidgetsHeaders.7z
  wget -q https://github.com/wxWidgets/wxWidgets/releases/download/v%WX_VERSION%/wxMSW-%WX_MSW_VERSION%_vc14x%WX_ARCH%_ReleaseDLL.7z ^
      -O wxWidgetsDLL.7z
  wget -q https://github.com/wxWidgets/wxWidgets/releases/download/v%WX_VERSION%/wxMSW-%WX_MSW_VERSION%_vc14x%WX_ARCH%_Dev.7z ^
      -O wxWidgetsDev.7z
  7z i > nul 2>&1 || choco install -y 7zip
  7z x -aoa wxWidgetsHeaders.7z -o%WXWIN%
  7z x -aoa wxWidgetsDLL.7z -o%WXWIN%
  7z x -aoa wxWidgetsDev.7z -o%WXWIN%
  if not "%WX_ARCH%"=="_x64" ren "%WXWIN%\lib\vc14x_dll" vc_dll
)
set "PATH=%PATH%;%WXWIN%;%wxWidgets_LIB_DIR%"
