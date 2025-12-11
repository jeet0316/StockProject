#!/bin/bash

# StockSense Installation Script

set -e

echo "=========================================="
echo "  StockSense Installation Script"
echo "=========================================="
echo ""

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     MACHINE=Linux;;
    Darwin*)    MACHINE=Mac;;
    *)          MACHINE="UNKNOWN:${OS}"
esac

echo "Detected OS: $MACHINE"
echo ""

# Check for required tools
echo "Checking for required tools..."

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed."
    if [ "$MACHINE" = "Mac" ]; then
        echo "Install with: brew install cmake"
    elif [ "$MACHINE" = "Linux" ]; then
        echo "Install with: sudo apt-get install cmake"
    fi
    exit 1
fi
echo "✓ CMake found"

# Check C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: C++ compiler not found."
    if [ "$MACHINE" = "Mac" ]; then
        echo "Install Xcode Command Line Tools: xcode-select --install"
    elif [ "$MACHINE" = "Linux" ]; then
        echo "Install with: sudo apt-get install build-essential"
    fi
    exit 1
fi
echo "✓ C++ compiler found"

# Check for libcurl
if [ "$MACHINE" = "Mac" ]; then
    if ! brew list curl &> /dev/null 2>&1; then
        echo "Installing curl..."
        brew install curl
    fi
elif [ "$MACHINE" = "Linux" ]; then
    if ! dpkg -l | grep -q libcurl4-openssl-dev; then
        echo "Installing libcurl..."
        sudo apt-get update
        sudo apt-get install -y libcurl4-openssl-dev
    fi
fi
echo "✓ libcurl found"

# Check for rapidcsv
if [ ! -f "include/rapidcsv.h" ] && [ ! -f "rapidcsv.h" ]; then
    echo ""
    echo "WARNING: rapidcsv.h not found!"
    echo "Please download rapidcsv.h from: https://github.com/d99kris/rapidcsv"
    echo "Place it in the 'include/' directory or project root."
    echo ""
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    echo "✓ rapidcsv.h found"
fi

# Check for Python (optional)
if command -v python3 &> /dev/null; then
    echo "✓ Python3 found"
    PYTHON_AVAILABLE=true
else
    echo "⚠ Python3 not found (optional, for plotting)"
    PYTHON_AVAILABLE=false
fi

# Install Python dependencies if Python is available
if [ "$PYTHON_AVAILABLE" = true ]; then
    if [ -f "scripts/requirements.txt" ]; then
        echo ""
        read -p "Install Python dependencies? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            pip3 install -r scripts/requirements.txt
            echo "✓ Python dependencies installed"
        fi
    fi
fi

# Create build directory
echo ""
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo ""
echo "Configuring project with CMake..."
cmake ..

# Build
echo ""
echo "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "=========================================="
echo "  Installation Complete!"
echo "=========================================="
echo ""
echo "To run StockSense:"
echo "  cd build"
echo "  ./StockSense"
echo ""
echo "Or from project root:"
echo "  ./build/StockSense"
echo ""

