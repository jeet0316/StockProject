#!/bin/bash

# Simple script to start the Flask website

echo "=========================================="
echo "  Starting StockSense Web Interface..."
echo "=========================================="
echo ""

cd "$(dirname "$0")"

# Check if Flask is installed
if ! python3 -c "import flask" 2>/dev/null; then
    echo "Installing Flask..."
    pip3 install flask
fi

echo "Starting Flask server on port 8080..."
echo ""
echo "🌐 Open your browser and go to:"
echo "   http://localhost:8080"
echo ""
echo "Press Ctrl+C to stop the server"
echo "=========================================="
echo ""

python3 app.py

