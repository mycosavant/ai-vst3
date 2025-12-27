# Installation Guide

Complete installation instructions for OBSIDIAN-Neural VST3 plugin.

## Prerequisites

### System Requirements

- **Windows:** 10/11 x64
- **macOS:** 10.15+ (Intel & Apple Silicon)
- **Linux:** Ubuntu 20.04+ x64
- **RAM:** 16GB+ recommended for local mode
- **Storage:** ~3GB for models and dependencies

### Stability AI Access

1. Create account on [Hugging Face](https://huggingface.co/)
2. Request access to [Stable Audio Open](https://huggingface.co/stabilityai/stable-audio-open-small)
3. Wait for approval (usually minutes to hours)
4. Generate access token in HF settings

---

## Option 1: Local Models (Windows only)

**Completely offline. No servers, Python, or GPU needed.**

### Step 1: Download Models

1. Get approval for Stable Audio Open (see Prerequisites)
2. Download all files from [innermost47/stable-audio-open-small-tflite](https://huggingface.co/innermost47/stable-audio-open-small-tflite)
3. Copy models to: `%APPDATA%\OBSIDIAN-Neural\stable-audio\` (Windows)

### Step 2: Install VST3

1. Download VST3 from [Releases](https://github.com/innermost47/ai-dj/releases)
2. Extract and copy to VST3 folder:
   - `C:\Program Files\Common Files\VST3\`

### Step 3: Configure Plugin

1. Load OBSIDIAN-Neural in your DAW
2. Choose "Local Model" option
3. Plugin will automatically find models in AppData folder

### Current Limitations

- Fixed 10-second generation
- Some timing/quantization issues
- High RAM usage during generation

---

## Option 2: Build from Source

**Full control with local server - for developers and advanced users.**

### Prerequisites

#### All Platforms
- **Python 3.10+** from [python.org](https://python.org)
- **Git** for cloning repository
- **CMake** for building VST3 plugin

#### Platform-Specific Requirements

**Windows:**
- [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/) with "Desktop development with C++" workload
- Optional: [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) for GPU acceleration

**macOS:**
- Xcode Command Line Tools: `xcode-select --install`
- Optional: [Homebrew](https://brew.sh/) for package management

**Linux:**
- Build essentials and audio libraries:
  ```bash
  # Ubuntu/Debian
  sudo apt install build-essential cmake git python3 python3-venv python3-pip libasound2-dev
  
  # Fedora
  sudo dnf groupinstall 'Development Tools'
  sudo dnf install cmake git python3 python3-pip alsa-lib-devel
  
  # Arch
  sudo pacman -S base-devel cmake git python python-pip alsa-lib
  ```
- Optional: NVIDIA CUDA Toolkit for GPU acceleration

### Installation Scripts

We provide automated installation scripts for each platform that handle all the setup:

#### Windows

```cmd
git clone https://github.com/innermost47/ai-dj.git
cd ai-dj
install-win.bat
```

#### macOS

```bash
git clone https://github.com/innermost47/ai-dj.git
cd ai-dj
chmod +x install-mac.sh
./install-mac.sh
```

#### Linux

```bash
git clone https://github.com/innermost47/ai-dj.git
cd ai-dj
chmod +x install-lnx.sh
./install-lnx.sh
```

### What the Installation Scripts Do

1. **Verify Prerequisites**
   - Check for Python, CMake, and Git
   - Display detected versions
   - Provide installation instructions if missing

2. **Create Python Environment**
   - Set up isolated virtual environment
   - Upgrade pip to latest version
   - Prevent conflicts with system Python

3. **Detect Hardware Acceleration**
   - **Windows/Linux:** Detect NVIDIA CUDA support
   - **macOS (Apple Silicon):** Configure Metal Performance Shaders (MPS)
   - **macOS (Intel):** CPU-only configuration
   - Install appropriate PyTorch version

4. **Install Dependencies**
   - PyTorch with GPU support (if available)
   - llama-cpp-python with hardware acceleration
   - FastAPI, Stable Audio Tools, and all required libraries
   - Platform-specific packages (excludes pywin32 on macOS/Linux)

5. **Build Plugin**
   - Compile VST3 plugin from source
   - **macOS:** Also builds Audio Unit (AU) format
   - Place build artifacts in `vst/build/` directory

### Post-Installation Setup

#### 1. Install Plugin Files

**Windows:**
```cmd
copy vst\build\*_artefacts\Release\VST3\*.vst3 "%CommonProgramFiles%\VST3\"
```

**macOS:**
```bash
# VST3
cp -r vst/build/*_artefacts/Release/VST3/*.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Audio Unit (for Logic Pro, GarageBand, etc.)
cp -r vst/build/*_artefacts/Release/AU/*.component ~/Library/Audio/Plug-Ins/Components/
```

**Linux:**
```bash
mkdir -p ~/.vst3
cp -r vst/build/*_artefacts/Release/VST3/*.vst3 ~/.vst3/
```

#### 2. Launch Server Interface

**Windows:**
```cmd
env\Scripts\activate.bat
python server_interface.py
```

**macOS/Linux:**
```bash
source env/bin/activate
python server_interface.py
```

**Features:**
- System tray integration with green triangle icon
- Real-time server status and controls
- Configuration management with API keys
- Secure Hugging Face token storage
- Live server logs with color coding
- First-time setup wizard

#### 3. Configure Server

- **First launch:** Setup wizard guides through configuration
- **Hugging Face Token:** Enter approved token (verification available)
- **API Keys:** Generate with credit limits or unlimited access

#### 4. Start Server

- Click "Start" in server interface
- **Authentication prompt:**
  - **Yes:** Use API keys (for production/network access)
  - **No:** Development bypass (for localhost only)
- Note the server URL (usually `http://localhost:8000`)

#### 5. Configure Plugin

- Load OBSIDIAN-Neural in your DAW
- **Server URL:** Paste from server GUI
- **API Key:** Copy from server interface (if using authentication)

### Troubleshooting

#### Installation Issues

**"Python not found"**
- Ensure Python 3.10+ is installed
- Windows: Check "Add Python to PATH" during installation
- macOS/Linux: Use `python3` command

**"CMake not found"**
- Windows: Download from [cmake.org](https://cmake.org/download/)
- macOS: `brew install cmake`
- Linux: Install via package manager (see prerequisites)

**"Git not found"**
- Windows: Download from [git-scm.com](https://git-scm.com/)
- macOS: `xcode-select --install`
- Linux: Install via package manager

#### GPU Acceleration Issues

**NVIDIA (Windows/Linux):**
- Verify CUDA installation: `nvidia-smi`
- Check CUDA version compatibility
- Update GPU drivers if needed

**Apple Silicon (macOS):**
- Metal support is automatic on M1/M2/M3
- Ensure macOS 12.3+ for best performance

**AMD (Linux only):**
- ROCm support requires manual configuration
- Consider CPU mode for simplicity

#### Build Failures

**Windows:**
- Install Visual Studio Build Tools with C++ workload
- Run installation script as administrator if needed

**macOS:**
- Install Xcode Command Line Tools: `xcode-select --install`
- For signing issues, build may complete but need manual code signing

**Linux:**
- Install missing development packages (see prerequisites)
- Check for audio library dependencies: `libasound2-dev` or `alsa-lib-devel`

#### Runtime Issues

- **No Hugging Face access:** Must be approved for Stable Audio Open first
- **Connection failed:** Ensure server is running before configuring VST
- **API confusion:** Choose "No" authentication for simple localhost setup
- **Port conflicts:** Server default is 8000, change in config if needed

### Performance Optimization

#### GPU Acceleration Status

The installation script automatically detects and configures:

- **NVIDIA CUDA (Windows/Linux):**
  - PyTorch with CUDA 12.4 support
  - llama-cpp-python with cuBLAS
  - bitsandbytes for 8-bit quantization

- **Apple Metal (macOS M1/M2/M3):**
  - PyTorch with MPS backend
  - llama-cpp-python with Metal acceleration
  - Optimized for Apple Silicon architecture

- **CPU Fallback (All platforms):**
  - Automatic if no GPU detected
  - Slower but fully functional

#### Memory Management

- **16GB+ RAM:** Recommended for stable operation
- **8GB RAM:** May work with reduced model size
- **Swap/Virtual Memory:** Ensure adequate if RAM limited

---

## Verification

### Test Installation

1. **Start the server** (see Post-Installation Setup)
2. **Load plugin** in your DAW
3. **Type simple prompt:** "techno kick"
4. **Click generate button**
5. **Wait for audio generation** (10-30 seconds)
6. **Play generated sample** via MIDI (C3-B3)

### Expected Behavior

- Generation progress shown in plugin UI
- Audio appears in waveform display
- Sample plays when triggered via MIDI
- DAW sync works with project tempo
- Server logs show generation activity

### Platform-Specific Notes

**macOS:**
- Both VST3 and AU formats available
- AU works in Logic Pro, GarageBand, MainStage
- VST3 works in most other DAWs

**Linux:**
- VST3 compatibility varies by DAW
- Test with REAPER, Bitwig, or Ardour
- May require audio server configuration (JACK/PipeWire)

---

## Uninstallation

### Remove Plugin Files

**Windows:**
- Delete from: `C:\Program Files\Common Files\VST3\`
- Remove: `%APPDATA%\OBSIDIAN-Neural\`

**macOS:**
- Delete VST3: `~/Library/Audio/Plug-Ins/VST3/OBSIDIAN-Neural.vst3`
- Delete AU: `~/Library/Audio/Plug-Ins/Components/OBSIDIAN-Neural.component`
- Remove config: `~/Library/Application Support/OBSIDIAN-Neural/`

**Linux:**
- Delete from: `~/.vst3/`
- Remove config: `~/.config/OBSIDIAN-Neural/`

### Server Installation

1. Stop server process
2. Deactivate virtual environment: `deactivate`
3. Delete cloned repository folder
4. Remove downloaded models from cache

---

## Getting Help

### Documentation

- **Tutorial:** [YouTube Video](https://youtu.be/-qdFo_PcKoY)
- **Community:** [GitHub Discussions](https://github.com/innermost47/ai-dj/discussions)
- **Documentation:** [obsidian-neural.com/documentation.html](https://obsidian-neural.com/documentation.html)

### Support

- **Issues:** [GitHub Issues](https://github.com/innermost47/ai-dj/issues)
- **Server Logs:** Check server interface for detailed error messages
- **Installation Logs:** Review console output from installation scripts

### Before Reporting Issues

Include the following information:

- Operating system and version
- Platform architecture (Intel/ARM/Apple Silicon)
- DAW name and version
- Installation method and script used
- GPU type and drivers (if applicable)
- Complete error messages from installation
- Server logs if runtime issue
- Steps to reproduce problem

---

## Quick Reference

### Start Server (After Installation)

**Windows:**
```cmd
cd path\to\ai-dj
env\Scripts\activate.bat
python server_interface.py
```

**macOS/Linux:**
```bash
cd path/to/ai-dj
source env/bin/activate
python server_interface.py
```

### Default Paths

| Platform | VST3 Location | Config Location |
|----------|--------------|-----------------|
| Windows | `C:\Program Files\Common Files\VST3\` | `%APPDATA%\OBSIDIAN-Neural\` |
| macOS | `~/Library/Audio/Plug-Ins/VST3/` | `~/Library/Application Support/OBSIDIAN-Neural/` |
| Linux | `~/.vst3/` | `~/.config/OBSIDIAN-Neural/` |

**macOS AU:** `~/Library/Audio/Plug-Ins/Components/`

---

_For the latest installation instructions and troubleshooting, always check the main repository._