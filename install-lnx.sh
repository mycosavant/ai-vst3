#!/bin/bash

# =============================================================================
# OBSIDIAN-Neural - Linux Installation Script
# =============================================================================
# This script installs OBSIDIAN-Neural in the current directory
# Prerequisites: Git repository must be cloned manually first
# =============================================================================

set -e  # Exit on error

echo ""
echo "==============================================="
echo "  OBSIDIAN-Neural - Linux Installation"
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
    echo "Install Python with your package manager:"
    echo "  Ubuntu/Debian: sudo apt install python3 python3-venv python3-pip"
    echo "  Fedora: sudo dnf install python3 python3-pip"
    echo "  Arch: sudo pacman -S python python-pip"
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
    echo "Install CMake with your package manager:"
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  Fedora: sudo dnf install cmake"
    echo "  Arch: sudo pacman -S cmake"
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
    echo "Install Git with your package manager:"
    echo "  Ubuntu/Debian: sudo apt install git"
    echo "  Fedora: sudo dnf install git"
    echo "  Arch: sudo pacman -S git"
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

# Detect CUDA
echo "[Step 7/8] Detecting GPU and installing Python dependencies..."
echo ""
CUDA_AVAILABLE=0
if command -v nvidia-smi &> /dev/null; then
    if nvidia-smi &> /dev/null; then
        CUDA_AVAILABLE=1
        echo "   NVIDIA GPU detected with CUDA support!"
        CUDA_VERSION=$(nvidia-smi | grep "CUDA Version" | awk '{print $9}')
        echo "   CUDA Version: $CUDA_VERSION"
        echo ""
    fi
fi

# Install PyTorch based on GPU detection
if [ $CUDA_AVAILABLE -eq 1 ]; then
    echo "Installing PyTorch with CUDA support..."
    echo "This may take several minutes..."
    python -m pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu124
    if [ $? -ne 0 ]; then
        echo "[ERROR] PyTorch CUDA installation failed!"
        exit 1
    fi
    echo "   PyTorch CUDA installed"
    echo ""
    
    echo "Installing llama-cpp-python with CUDA support..."
    CMAKE_ARGS="-DGGML_CUDA=on" python -m pip install llama-cpp-python --no-cache-dir --verbose
    if [ $? -ne 0 ]; then
        echo "[ERROR] llama-cpp-python CUDA installation failed!"
        exit 1
    fi
    echo "   llama-cpp-python CUDA installed"
    echo ""
    
    echo "Installing bitsandbytes for 8-bit quantization..."
    python -m pip install bitsandbytes || echo "   [WARNING] bitsandbytes installation failed, continuing..."
    echo ""
else
    echo "Installing PyTorch (CPU only)..."
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

# Build VST plugin
echo "[Step 8/8] Building VST3 plugin..."
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
    echo "Make sure you have build essentials installed:"
    echo "  Ubuntu/Debian: sudo apt install build-essential libasound2-dev"
    echo "  Fedora: sudo dnf groupinstall 'Development Tools' && sudo dnf install alsa-lib-devel"
    echo "  Arch: sudo pacman -S base-devel alsa-lib"
    echo ""
    cd ../..
    exit 1
fi
echo "   CMake configuration successful"
echo ""

# Build VST
echo "Compiling VST plugin..."
echo "This may take several minutes..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    echo "[ERROR] VST compilation failed!"
    echo ""
    cd ../..
    exit 1
fi
echo "   VST plugin compiled successfully"
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
echo "The VST3 plugin can be found in: vst/build/"
echo "Copy the .vst3 directory to: ~/.vst3/"
echo ""
echo ""