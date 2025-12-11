# StockSense: Live Stock Price Viewer

A modern web application for viewing live stock prices with interactive graphs, similar to Robinhood.

## Features

- 📈 **Live Stock Prices** - Real-time stock quotes from Yahoo Finance API
- 📊 **Interactive Graphs** - Smooth, Robinhood-style charts with multiple time periods
- 🎨 **Visual Indicators** - Green ↑ for price increases, Red ↓ for decreases
- ⚡ **Auto-Refresh** - Prices update automatically every 30 seconds
- 🌙 **Dark Theme** - Modern black background design

## Quick Start

### 1. Install Dependencies

```bash
cd scripts
pip3 install -r requirements.txt
```

### 2. Run the Server

```bash
python3 app.py
```

Or use the start script:
```bash
./start_website.sh
```

### 3. Open in Browser

Go to: **http://localhost:8080**

Enter any stock ticker (e.g., AAPL, GOOGL, MSFT, TSLA) and view live prices with interactive graphs!

## Usage

1. Open http://localhost:8080 in your browser
2. Enter a stock ticker in the search bar
3. View live price, change, and interactive graph
4. Click time period buttons (1D, 5D, 1M, 3M, 6M, 1Y, 5Y) to change the graph view

## Project Structure

```
StockSense-data-input/
├── scripts/
│   ├── app.py              # Flask server
│   ├── templates/
│   │   └── stock_viewer.html  # Main web interface
│   ├── requirements.txt    # Python dependencies
│   └── start_website.sh   # Start script
├── src/                    # C++ source code (for class project)
├── README.md              # This file
└── config.json            # Configuration
```

## Requirements

- Python 3.7+
- Flask
- requests

Install with: `pip3 install -r scripts/requirements.txt`

## Authors

Brandon Aulac, Isadora Selime, Arya Shenoy, Jeetkumar Desai, Sri Kiran Sripada

## License

Educational project for C++ class.
