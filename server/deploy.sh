#!/bin/bash

# Install Node.js if not present
if ! command -v node &> /dev/null; then
    curl -fsSL https://deb.nodesource.com/setup_20.x | bash -
    apt-get install -y nodejs
fi

# Create app directory
mkdir -p /opt/dev-planner-server
cd /opt/dev-planner-server

# Install dependencies and build
npm install
npm run build

echo "Server installed successfully"
