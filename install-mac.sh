#!/bin/bash

# =============================================================================
# OBSIDIAN-Neural - macOS Installation Script
# =============================================================================
# This script installs OBSIDIAN-Neural in the current directory
# Prerequisites: Git repository must be cloned manually first
# =============================================================================

set -e  # Exit on error

echo ""
echo "==============================================="
echo "  OBSIDIAN-Neural - macOS Installation"
echo "==============================================="
echo ""

# Check we're in the right directory
if [ ! -f "main.py" ]; then
    echo "[ERROR] main.py not found!"
    echo ""
    echo "Make sure you run this script from the cloned project directory."
    echo ""
    exit 1
fi

# Check if Python is installed
echo "[Step 1/8] Checking Python..."
if ! command -v python3 &> /dev/null; then
    echo "[ERROR] Python3 is not installed!"
    echo ""
    echo "Install Python from: https://www.python.org/downloads/"
    echo "Or use Homebrew: brew install python3"
    echo ""
    exit 1
fi

PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
echo "   Python $PYTHON_VERSION detected"
echo ""

# Check if CMake is installed
echo "[Step 2/8] Checking CMake..."
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake is not installed!"
    echo ""
    echo "Install CMake with Homebrew: brew install cmake"
    echo "Or download from: https://cmake.org/download/"
    echo ""
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
echo "   CMake $CMAKE_VERSION detected"
echo ""

# Check if Git is installed
echo "[Step 3/8] Checking Git..."
if ! command -v git &> /dev/null; then
    echo "[ERROR] Git is not installed!"
    echo ""
    echo "Install Git with: xcode-select --install"
    echo "Or with Homebrew: brew install git"
    echo ""
    exit 1
fi

GIT_VERSION=$(git --version | awk '{print $3}')
echo "   Git $GIT_VERSION detected"
echo ""

# Create necessary directories
echo "[Step 4/8] Creating directories..."
mkdir -p models
echo "   models directory created"
echo ""

# Create virtual environment
echo "[Step 5/8] Creating Python virtual environment..."
if [ -d "env" ]; then
    echo "   Existing virtual environment detected, cleaning up..."
    rm -rf env
fi

python3 -m venv env
echo "   Virtual environment created successfully"
echo ""

# Activate virtual environment
echo "[Step 6/8] Activating virtual environment..."
source env/bin/activate
echo "   Virtual environment activated"
echo ""

# Upgrade pip
echo "   Upgrading pip..."
python -m pip install --upgrade pip || echo "   [WARNING] Failed to upgrade pip, continuing anyway..."
echo ""

# Detect Apple Silicon vs Intel
echo "[Step 7/8] Detecting architecture and installing Python dependencies..."
echo ""
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
    echo "   Apple Silicon (M1/M2/M3) detected!"
    echo ""
    
    echo "Installing PyTorch with Metal Performance Shaders (MPS) support..."
    echo "This may take several minutes..."
    python -m pip install torch torchvision torchaudio
    if [ $? -ne 0 ]; then
        echo "[ERROR] PyTorch installation failed!"
        exit 1
    fi
    echo "   PyTorch with MPS installed"
    echo ""
    
    echo "Installing llama-cpp-python with Metal support..."
    CMAKE_ARGS="-DGGML_METAL=on" python -m pip install llama-cpp-python --no-cache-dir
    if [ $? -ne 0 ]; then
        echo "[ERROR] llama-cpp-python Metal installation failed!"
        exit 1
    fi
    echo "   llama-cpp-python Metal installed"
    echo ""
    
else
    echo "   Intel Mac detected"
    echo ""
    
    echo "Installing PyTorch (CPU)..."
    echo "This may take several minutes..."
    python -m pip install torch torchvision torchaudio
    if [ $? -ne 0 ]; then
        echo "[ERROR] PyTorch installation failed!"
        exit 1
    fi
    echo "   PyTorch CPU installed"
    echo ""
    
    echo "Installing llama-cpp-python (CPU)..."
    python -m pip install llama-cpp-python==0.3.9
    if [ $? -ne 0 ]; then
        echo "[ERROR] llama-cpp-python installation failed!"
        exit 1
    fi
    echo "   llama-cpp-python CPU installed"
    echo ""
fi

# Install main dependencies
echo "Installing main libraries..."
echo ""

PACKAGES="diffusers transformers accelerate librosa soundfile fastapi uvicorn python-dotenv requests apscheduler demucs cryptography pyinstaller pystray psutil Pillow"

for package in $PACKAGES; do
    echo "   Installing $package..."
    python -m pip install $package || echo "   [WARNING] Error installing $package"
done

echo ""
echo "==============================================="
echo "  Python dependencies installed successfully!"
echo "==============================================="
echo ""

# Build VST/AU plugins
echo "[Step 8/8] Building VST3 and AU plugins..."
echo ""

if [ ! -d "vst" ]; then
    echo "[ERROR] VST source directory not found!"
    echo ""
    exit 1
fi

if [ ! -f "vst/CMakeLists.txt" ]; then
    echo "[ERROR] CMakeLists.txt not found in vst directory!"
    echo ""
    exit 1
fi

# Clean previous build
if [ -d "vst/build" ]; then
    echo "Cleaning previous build cache..."
    rm -rf vst/build
fi

# Create build directory
mkdir -p vst/build
cd vst/build

# Configure CMake
echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "[ERROR] CMake configuration failed!"
    echo ""
    echo "Make sure you have Xcode Command Line Tools installed:"
    echo "  xcode-select --install"
    echo ""
    cd ../..
    exit 1
fi
echo "   CMake configuration successful"
echo ""

# Build plugins
echo "Compiling VST3 and AU plugins..."
echo "This may take several minutes..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    echo "[ERROR] Plugin compilation failed!"
    echo ""
    cd ../..
    exit 1
fi
echo "   Plugins compiled successfully"
echo ""

cd ../..

echo ""
echo "==============================================="
echo "  Installation completed successfully!"
echo "==============================================="
echo ""
echo "To use OBSIDIAN-Neural:"
echo "  1. Activate the environment: source env/bin/activate"
echo "  2. Start the server: python main.py"
echo ""
echo "Plugins can be found in: vst/build/"
echo ""
echo "Installation instructions:"
echo "  VST3: Copy the .vst3 bundle to ~/Library/Audio/Plug-Ins/VST3/"
echo "  AU:   Copy the .component bundle to ~/Library/Audio/Plug-Ins/Components/"
echo ""
echo "Quick install command:"
echo "  cp -r vst/build/*_artefacts/Release/VST3/*.vst3 ~/Library/Audio/Plug-Ins/VST3/"
echo "  cp -r vst/build/*_artefacts/Release/AU/*.component ~/Library/Audio/Plug-Ins/Components/"
echo ""
echo ""