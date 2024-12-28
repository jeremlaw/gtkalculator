#!/bin/bash

# Check for the operating system and install the dependencies accordingly.

# Function to install dependencies for Linux (Ubuntu/Debian)
install_linux() {
    echo "Installing dependencies for Linux..."

    sudo apt update # Update package list

    sudo apt install -y libgtk-4-dev # Install GTK4

    echo "GTK4 and dependencies installed successfully."
}

# Function to install dependencies for macOS
install_macos() {
    echo "Installing dependencies for macOS..."

    # Install Homebrew if not already installed
    if ! command -v brew &>/dev/null; then
        echo "Homebrew not found, installing..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi

    brew install gtk4 # Install GTK4 via Homebrew

    echo "GTK4 and dependencies installed successfully on macOS."
}

# Function to handle Linux or macOS based on the operating system
install_dependencies() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        install_linux
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        install_macos
    else
        echo "Unsupported operating system. Please install GTK4 manually."
        exit 1
    fi
}

install_dependencies # Run the installation