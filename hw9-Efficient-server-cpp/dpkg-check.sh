#!/bin/bash

# List of packages to check
packages=(
    build-essential
    netcat-openbsd
    pkg-config
    make
    wget
    curl
    git
    cmake
    libfmt-dev
    libboost-all-dev
    protobuf-compiler
    libprotobuf-dev
    libprotoc-dev
)

# Function to check if a package is installed
check_package() {
    dpkg -l | grep -qw "$1"
}

# Iterate over the packages and check their presence
for package in "${packages[@]}"; do
    if check_package "$package"; then
        echo "Package $package is installed."
    else
        echo "Package $package is NOT installed."
    fi
done
