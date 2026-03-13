#!/bin/bash
# ===============================================
# Unix Install and Setup Script (Mac & Ubuntu)
# ===============================================

set -e

# Resolve the directory this script lives in
SOURCE_DIR="$(cd "$(dirname "$0")" && pwd)"

echo ""
echo "==============================================="
echo "  GAM_200 Engine Installer - Unix"
echo "==============================================="
echo ""

# Helper: check command exists
command_exists() { command -v "$1" >/dev/null 2>&1; }

# -----------------------------------------------
# Select installation directory
# -----------------------------------------------
echo "Selecting installation folder..."
echo "Default: $SOURCE_DIR"
echo ""

INSTALL_DIR=""

# Try GUI folder pickers: zenity (GNOME), kdialog (KDE), osascript (macOS)
if command_exists zenity; then
    INSTALL_DIR=$(zenity --file-selection \
        --directory \
        --title="Select installation folder for GAM_200 Engine" \
        --filename="$SOURCE_DIR/" 2>/dev/null) || INSTALL_DIR=""
elif command_exists kdialog; then
    INSTALL_DIR=$(kdialog --getexistingdirectory "$SOURCE_DIR" \
        --title "Select installation folder for GAM_200 Engine" 2>/dev/null) || INSTALL_DIR=""
elif [[ "$(uname)" == "Darwin" ]]; then
    INSTALL_DIR=$(osascript -e \
        "POSIX path of (choose folder with prompt \"Select installation folder for GAM_200 Engine\" default location POSIX file \"$SOURCE_DIR\")" \
        2>/dev/null | sed 's|/$||') || INSTALL_DIR=""
fi

# Fallback: text prompt
if [ -z "$INSTALL_DIR" ]; then
    echo "No GUI folder picker available (or dialog cancelled)."
    echo "Enter installation path, or press Enter to use default:"
    printf "  [%s]: " "$SOURCE_DIR"
    read -r USER_INPUT
    if [ -z "$USER_INPUT" ]; then
        INSTALL_DIR="$SOURCE_DIR"
    else
        INSTALL_DIR="${USER_INPUT/#\~/$HOME}"
    fi
fi

echo ""
echo "Install location: $INSTALL_DIR"
echo ""

# If the user picked a different folder, copy the engine distribution there first
if [ "$INSTALL_DIR" != "$SOURCE_DIR" ]; then
    echo "Copying engine files to selected folder..."
    mkdir -p "$INSTALL_DIR"

    # Engine source directories (Scripts = C# scripting project)
    for DIR in Engine Extern Shaders cmake Logs mono Scripts; do
        if [ -d "$SOURCE_DIR/$DIR" ]; then
            cp -r "$SOURCE_DIR/$DIR" "$INSTALL_DIR/"
            echo "   Copied: $DIR/"
        fi
    done

    # Loose root files
    for FILE in CMakeLists.txt mono-2.0-sgen.dll MonoPosixHelper.dll script_build.sh script_clean.sh script_run.sh; do
        if [ -f "$SOURCE_DIR/$FILE" ]; then
            cp "$SOURCE_DIR/$FILE" "$INSTALL_DIR/$FILE"
            echo "   Copied: $FILE"
        fi
    done

    echo "   Engine files copied."
    echo ""
fi

# All subsequent steps run inside the chosen install directory
cd "$INSTALL_DIR"

# -----------------------------------------------
# Pre-flight checks
# -----------------------------------------------
if ! command_exists cmake; then
    echo "ERROR: CMake is not installed or not in PATH"
    echo "  Ubuntu/Debian: sudo apt-get install cmake"
    echo "  macOS:         brew install cmake"
    exit 1
fi

if ! command_exists g++ && ! command_exists clang++; then
    echo "WARNING: No C++ compiler found in PATH"
    echo "  Ubuntu/Debian: sudo apt-get install build-essential"
    echo "  macOS:         xcode-select --install"
    echo ""
fi

# -----------------------------------------------
# Step 1: Create Asset folder structure
# -----------------------------------------------
echo "[1/6] Setting up Assets directory structure..."

mkdir -p Assets
echo "   Assets/ ready"

for DIR in Audio Prefabs Replay Scenes Scripts Sprites Tiles; do
    if [ ! -d "Assets/$DIR" ]; then
        mkdir -p "Assets/$DIR"
        echo "   Created: Assets/$DIR/"
    else
        echo "   Already exists: Assets/$DIR/"
    fi
done

# Copy GameAPI.cs into Assets/Scripts (skip if installing in-place to avoid self-copy zeroing the file)
if [ "$INSTALL_DIR" != "$SOURCE_DIR" ]; then
    if [ -f "$SOURCE_DIR/Assets/Scripts/GameAPI.cs" ]; then
        cp "$SOURCE_DIR/Assets/Scripts/GameAPI.cs" "Assets/Scripts/GameAPI.cs"
        echo "   Copied: GameAPI.cs -> Assets/Scripts/GameAPI.cs"
    else
        echo "   WARNING: GameAPI.cs not found at $SOURCE_DIR/Assets/Scripts/, skipping"
    fi
else
    echo "   GameAPI.cs already in place"
fi

# -----------------------------------------------
# Step 2: Copy Editor_Icons
# -----------------------------------------------
echo ""
echo "[2/6] Copying Editor_Icons..."

mkdir -p "Assets/Editor_Icons"

if [ -d "Logs/default/Editor_Icons" ]; then
    cp -r "Logs/default/Editor_Icons/." "Assets/Editor_Icons/"
    echo "   Copied Editor_Icons from Logs/default/Editor_Icons"
else
    echo "ERROR: Logs/default/Editor_Icons not found (checked in $INSTALL_DIR/Logs/default/)"
    exit 1
fi

ICON_COUNT=$(find "Assets/Editor_Icons" -maxdepth 1 -name "*.png" | wc -l | tr -d ' ')
echo "   Icons found: $ICON_COUNT PNG file(s)"

# -----------------------------------------------
# Step 3: Set up Game folder with default main.cpp / main.h
# -----------------------------------------------
echo ""
echo "[3/6] Setting up Game folder..."

mkdir -p Game
echo "   Game/ directory ready"

# Always copy — fail loudly if the source is missing
if [ -f "Logs/default/main.cpp" ]; then
    cp "Logs/default/main.cpp" "Game/main.cpp"
    echo "   Copied: Logs/default/main.cpp -> Game/main.cpp"
else
    echo "ERROR: Logs/default/main.cpp not found (checked in $INSTALL_DIR/Logs/default/)"
    exit 1
fi

if [ -f "Logs/default/main.h" ]; then
    cp "Logs/default/main.h" "Game/main.h"
    echo "   Copied: Logs/default/main.h -> Game/main.h"
else
    echo "ERROR: Logs/default/main.h not found (checked in $INSTALL_DIR/Logs/default/)"
    exit 1
fi

# -----------------------------------------------
# Step 4: Set up Scripts folder (C# scripting project)
# -----------------------------------------------
echo ""
echo "[4/6] Setting up Scripts folder..."

if [ -d "Scripts" ]; then
    echo "   Scripts/ already exists"
elif [ -d "$SOURCE_DIR/Scripts" ]; then
    cp -r "$SOURCE_DIR/Scripts" "Scripts"
    echo "   Copied Scripts from $SOURCE_DIR/Scripts"
else
    echo "   WARNING: Scripts folder not found at $SOURCE_DIR/Scripts, skipping"
fi

# -----------------------------------------------
# Step 5: Configure Engine with CMake
# -----------------------------------------------
echo ""
echo "[5/6] Configuring project with CMake..."

mkdir -p Build
cd Build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    cd ..
    exit 1
fi
cd ..

# -----------------------------------------------
# Step 6: Build the Engine
# -----------------------------------------------
echo ""
echo "[6/6] Building Engine with parallel compilation..."

cd Build
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build . --config Debug --parallel "$JOBS"
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    cd ..
    exit 1
fi
cd ..

echo ""
echo "==============================================="
echo "  Installation completed successfully!"
echo "==============================================="
echo ""
echo "  Location:            $INSTALL_DIR"
echo "  Assets/              - Asset directories created"
echo "  Assets/Editor_Icons/ - Editor icons ready"
echo "  Game/main.cpp        - Default entry point ready"
echo "  Game/main.h          - Default header ready"
echo "  Scripts/             - C# scripting project ready"
echo "  Build/               - CMake build configured"
echo ""
