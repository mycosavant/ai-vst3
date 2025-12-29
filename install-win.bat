@echo off
REM =============================================================================
REM OBSIDIAN-Neural - Windows Installation Script
REM =============================================================================
REM This script installs OBSIDIAN-Neural in the current directory
REM Prerequisites: Git repository must be cloned manually first
REM =============================================================================

setlocal enabledelayedexpansion

echo.
echo ===============================================
echo   OBSIDIAN-Neural - Windows Installation
echo ===============================================
echo.

REM Check we're in the right directory
if not exist "main.py" (
    echo [ERROR] main.py not found!
    echo.
    echo Make sure you run this script from the cloned project directory.
    echo.
    pause
    exit /b 1
)

REM Check if Python is installed
echo [Step 1/9] Checking Python...
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python is not installed or not in PATH!
    echo.
    echo Download Python from: https://www.python.org/downloads/
    echo Don't forget to check "Add Python to PATH" during installation.
    echo.
    pause
    exit /b 1
)

for /f "tokens=2" %%i in ('python --version 2^>^&1') do set PYTHON_VERSION=%%i
echo    Python %PYTHON_VERSION% detected
echo.

REM Check if CMake is installed
echo [Step 2/9] Checking CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not installed or not in PATH!
    echo.
    echo Download CMake from: https://cmake.org/download/
    echo Make sure to add CMake to system PATH during installation.
    echo.
    pause
    exit /b 1
)

for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"version"') do set CMAKE_VERSION=%%i
echo    CMake %CMAKE_VERSION% detected
echo.

REM Check if Git is installed
echo [Step 3/9] Checking Git...
git --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git is not installed or not in PATH!
    echo.
    echo Download Git from: https://git-scm.com/downloads
    echo.
    pause
    exit /b 1
)

for /f "tokens=3" %%i in ('git --version') do set GIT_VERSION=%%i
echo    Git %GIT_VERSION% detected
echo.

REM Create necessary directories
echo [Step 4/9] Creating directories...
if not exist "models" mkdir models
echo    models directory created
echo.

REM Download GGUF models
echo [Step 5/9] Downloading AI models...
echo.

if not exist "models\gemma-3-4b-it-Q4_K_M.gguf" (
    echo Downloading Gemma 3 4B model (approximately 2.5GB)...
    echo This may take several minutes depending on your connection...
    curl -L -o "models\gemma-3-4b-it-Q4_K_M.gguf" "https://huggingface.co/unsloth/gemma-3-4b-it-GGUF/resolve/main/gemma-3-4b-it-Q4_K_M.gguf?download=true"
    if errorlevel 1 (
        echo [ERROR] Failed to download Gemma 3 model!
        pause
        exit /b 1
    )
    echo    Gemma 3 4B model downloaded
) else (
    echo    Gemma 3 4B model already exists, skipping download
)
echo.

if not exist "models\ggml-model-Q4_K_M.gguf" (
    echo Downloading MiniCPM-V vision model (approximately 2.0GB)...
    curl -L -o "models\ggml-model-Q4_K_M.gguf" "https://huggingface.co/openbmb/MiniCPM-V-2_6-gguf/resolve/main/ggml-model-Q4_K_M.gguf?download=true"
    if errorlevel 1 (
        echo [ERROR] Failed to download MiniCPM-V model!
        pause
        exit /b 1
    )
    echo    MiniCPM-V vision model downloaded
) else (
    echo    MiniCPM-V vision model already exists, skipping download
)
echo.

if not exist "models\mmproj-model-f16.gguf" (
    echo Downloading MiniCPM-V projection model (approximately 600MB)...
    curl -L -o "models\mmproj-model-f16.gguf" "https://huggingface.co/openbmb/MiniCPM-V-2_6-gguf/resolve/main/mmproj-model-f16.gguf?download=true"
    if errorlevel 1 (
        echo [ERROR] Failed to download MiniCPM-V projection model!
        pause
        exit /b 1
    )
    echo    MiniCPM-V projection model downloaded
) else (
    echo    MiniCPM-V projection model already exists, skipping download
)
echo.

echo ===============================================
echo   AI models downloaded successfully!
echo ===============================================
echo.

REM Create virtual environment
echo [Step 6/9] Creating Python virtual environment...
if exist "env" (
    echo    Existing virtual environment detected, cleaning up...
    rmdir /s /q env
)

python -m venv env --copies
if errorlevel 1 (
    echo [ERROR] Failed to create virtual environment!
    echo.
    pause
    exit /b 1
)
echo    Virtual environment created successfully
echo.

REM Activate virtual environment
echo [Step 7/9] Activating virtual environment...
call env\Scripts\activate.bat
if errorlevel 1 (
    echo [ERROR] Failed to activate virtual environment!
    echo.
    pause
    exit /b 1
)
echo    Virtual environment activated
echo.

REM Upgrade pip
echo    Upgrading pip...
python -m pip install --upgrade pip >nul 2>&1
if errorlevel 1 (
    echo    [WARNING] Failed to upgrade pip, continuing anyway...
)
echo.

REM Detect CUDA
echo [Step 8/9] Detecting GPU and installing Python dependencies...
echo.
set CUDA_AVAILABLE=0
nvidia-smi >nul 2>&1
if not errorlevel 1 (
    set CUDA_AVAILABLE=1
    echo    NVIDIA GPU detected with CUDA support!
    echo.
)

REM Install PyTorch based on GPU detection
if %CUDA_AVAILABLE%==1 (
    echo Installing PyTorch with CUDA support...
    echo This may take several minutes...
    python -m pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu124
    if errorlevel 1 (
        echo [ERROR] PyTorch CUDA installation failed!
        pause
        exit /b 1
    )
    echo    PyTorch CUDA installed
    echo.
    
    echo Installing llama-cpp-python with CUDA support...
    python -m pip install llama-cpp-python --prefer-binary --extra-index-url=https://jllllll.github.io/llama-cpp-python-cuBLAS-wheels/AVX2/cu124
    if errorlevel 1 (
        echo [ERROR] llama-cpp-python CUDA installation failed!
        pause
        exit /b 1
    )
    echo    llama-cpp-python CUDA installed
    echo.
    
    echo Installing bitsandbytes for 8-bit quantization...
    python -m pip install bitsandbytes
    if errorlevel 1 (
        echo    [WARNING] bitsandbytes installation failed, continuing...
    )
    echo.
) else (
    echo Installing PyTorch CPU only...
    echo This may take several minutes...
    python -m pip install torch torchvision torchaudio
    if errorlevel 1 (
        echo [ERROR] PyTorch installation failed!
        pause
        exit /b 1
    )
    echo    PyTorch CPU installed
    echo.
    
    echo Installing llama-cpp-python CPU...
    python -m pip install llama-cpp-python==0.3.9
    if errorlevel 1 (
        echo [ERROR] llama-cpp-python installation failed!
        pause
        exit /b 1
    )
    echo    llama-cpp-python CPU installed
    echo.
)

REM Install main dependencies
echo Installing main libraries...
echo.

set PACKAGES=diffusers transformers accelerate librosa torchsde soundfile fastapi uvicorn python-dotenv requests apscheduler cryptography pystray psutil Pillow pywin32

for %%p in (%PACKAGES%) do (
    echo    Installing %%p...
    python -m pip install %%p >nul 2>&1
    if errorlevel 1 (
        echo    [WARNING] Error installing %%p
    )
)

echo.
echo ===============================================
echo   Python dependencies installed successfully!
echo ===============================================
echo.

REM Build VST plugin
echo [Step 9/9] Building VST3 plugin...
echo.

if not exist "vst" (
    echo [ERROR] VST source directory not found!
    echo.
    pause
    exit /b 1
)

if not exist "vst\CMakeLists.txt" (
    echo [ERROR] CMakeLists.txt not found in vst directory!
    echo.
    pause
    exit /b 1
)

REM Clean previous build
if exist "vst\build" (
    echo Cleaning previous build cache...
    rmdir /s /q vst\build
)

REM Create build directory
mkdir vst\build
cd vst\build

REM Configure CMake
echo Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    echo.
    echo Make sure you have Visual Studio Build Tools installed.
    echo Download from: https://visualstudio.microsoft.com/downloads/
    echo Select "Desktop development with C++" workload.
    echo.
    cd ..\..
    pause
    exit /b 1
)
echo    CMake configuration successful
echo.

REM Build VST
echo Compiling VST plugin...
echo This may take several minutes...
cmake --build . --config Release
if errorlevel 1 (
    echo [ERROR] VST compilation failed!
    echo.
    cd ..\..
    pause
    exit /b 1
)
echo    VST plugin compiled successfully
echo.

cd ..\..

echo.
echo ===============================================
echo   Installation completed successfully!
echo ===============================================
echo.
echo To use OBSIDIAN-Neural:
echo   1. Activate the environment: env\Scripts\activate.bat
echo   2. Start the server: python server_interface.py
echo.
echo The VST3 plugin can be found in: vst\build\
echo Copy the .vst3 file/folder to your DAW's VST3 directory
echo.
echo.
pause