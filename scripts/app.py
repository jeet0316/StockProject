#!/usr/bin/env python3
"""
Flask web interface for StockSense - Live Stock Prices with Graphs
Run with: python3 app.py
Then visit http://localhost:8080 in your browser
"""

from flask import Flask, render_template, jsonify, request
import requests
from pathlib import Path
from datetime import datetime, timedelta
import json

app = Flask(__name__)

OUTPUT_DIR = Path(__file__).parent.parent / "output"

def get_live_quote(ticker):
    """Fetch live stock quote from Yahoo Finance API."""
    try:
        # Use 5-day range to get more complete data
        url = f"https://query1.finance.yahoo.com/v8/finance/chart/{ticker}?interval=1d&range=5d"
        headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
        }
        response = requests.get(url, headers=headers, timeout=10)
        if response.status_code == 200:
            data = response.json()
            if 'chart' in data and 'result' in data['chart']:
                results = data['chart']['result']
                if results and len(results) > 0:
                    result = results[0]
                    meta = result.get('meta', {})
                    indicators = result.get('indicators', {})
                    quote = indicators.get('quote', [])
                    timestamps = result.get('timestamp', [])
                    
                    # Get current price
                    current_price = meta.get('regularMarketPrice', 0) or meta.get('chartPreviousClose', 0)
                    
                    # Get today's data from quote array (last entry)
                    if quote and len(quote) > 0:
                        quote_data = quote[0]
                        opens = quote_data.get('open', [])
                        highs = quote_data.get('high', [])
                        lows = quote_data.get('low', [])
                        volumes = quote_data.get('volume', [])
                        closes = quote_data.get('close', [])
                        
                        # Get the most recent (last) values
                        open_price = opens[-1] if opens and len(opens) > 0 and opens[-1] is not None else 0
                        high_price = max([h for h in highs if h is not None]) if highs else 0
                        low_price = min([l for l in lows if l is not None]) if lows else 0
                        volume_val = volumes[-1] if volumes and len(volumes) > 0 and volumes[-1] is not None else 0
                        current_close = closes[-1] if closes and len(closes) > 0 and closes[-1] is not None else current_price
                        
                        # Get previous close (second to last close, or from meta)
                        previous_close = meta.get('previousClose', 0)
                        if not previous_close and len(closes) > 1:
                            # Get the close from previous day
                            prev_closes = [c for c in closes if c is not None]
                            if len(prev_closes) > 1:
                                previous_close = prev_closes[-2]
                        
                        # If still no previous close, try chartPreviousClose
                        if not previous_close:
                            previous_close = meta.get('chartPreviousClose', 0)
                        
                        # Update current price if we have a better value
                        if current_close > 0:
                            current_price = current_close
                    else:
                        # Fallback to meta values
                        open_price = meta.get('open', 0) or meta.get('regularMarketOpen', 0)
                        high_price = meta.get('high', 0) or meta.get('regularMarketDayHigh', 0)
                        low_price = meta.get('low', 0) or meta.get('regularMarketDayLow', 0)
                        volume_val = meta.get('volume', 0) or meta.get('regularMarketVolume', 0)
                        previous_close = meta.get('previousClose', 0) or meta.get('chartPreviousClose', 0)
                    
                    change = current_price - previous_close if previous_close > 0 else 0
                    change_percent = (change / previous_close * 100) if previous_close > 0 else 0
                    
                    return {
                        'ticker': ticker,
                        'currentPrice': current_price,
                        'previousClose': previous_close,
                        'change': change,
                        'changePercent': change_percent,
                        'open': open_price,
                        'high': high_price,
                        'low': low_price,
                        'volume': int(volume_val) if volume_val else 0,
                        'lastUpdate': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                    }
    except Exception as e:
        print(f"Error fetching quote for {ticker}: {e}")
        import traceback
        traceback.print_exc()
    return None

def get_historical_data(ticker, period='1mo'):
    """Fetch historical stock data for charting."""
    try:
        # Map period to Yahoo Finance range
        period_map = {
            '1d': {'range': '1d', 'interval': '5m'},
            '5d': {'range': '5d', 'interval': '15m'},
            '1mo': {'range': '1mo', 'interval': '1d'},
            '3mo': {'range': '3mo', 'interval': '1d'},
            '6mo': {'range': '6mo', 'interval': '1d'},
            '1y': {'range': '1y', 'interval': '1d'},
            '5y': {'range': '5y', 'interval': '1wk'}
        }
        
        params = period_map.get(period, period_map['1mo'])
        url = f"https://query1.finance.yahoo.com/v8/finance/chart/{ticker}?interval={params['interval']}&range={params['range']}"
        
        headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
        }
        
        response = requests.get(url, headers=headers, timeout=10)
        if response.status_code == 200:
            data = response.json()
            if 'chart' in data and 'result' in data['chart']:
                results = data['chart']['result']
                if results and len(results) > 0:
                    result = results[0]
                    timestamps = result.get('timestamp', [])
                    indicators = result.get('indicators', {})
                    quote = indicators.get('quote', [])
                    
                    if quote and len(quote) > 0:
                        closes = quote[0].get('close', [])
                        
                        # Combine timestamps and prices
                        chart_data = []
                        for i, timestamp in enumerate(timestamps):
                            if i < len(closes) and closes[i] is not None:
                                chart_data.append({
                                    'time': datetime.fromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S'),
                                    'price': closes[i]
                                })
                        
                        return chart_data
    except Exception as e:
        print(f"Error fetching historical data for {ticker}: {e}")
        import traceback
        traceback.print_exc()
    return []

@app.route('/')
def index():
    """Main page - search for a stock."""
    return render_template('stock_viewer.html')

@app.route('/stock/<ticker>')
def view_stock(ticker):
    """View live stock price and graph."""
    quote = get_live_quote(ticker.upper())
    return render_template('stock_viewer.html', ticker=ticker.upper(), quote=quote)

@app.route('/api/quote/<ticker>')
def api_quote(ticker):
    """API endpoint for live stock quote."""
    quote = get_live_quote(ticker.upper())
    if quote:
        return jsonify(quote)
    return jsonify({'error': 'Could not fetch quote'}), 404

@app.route('/api/historical/<ticker>')
def api_historical(ticker):
    """API endpoint for historical stock data."""
    period = request.args.get('period', '1mo')
    data = get_historical_data(ticker.upper(), period)
    if data:
        return jsonify(data)
    return jsonify({'error': 'Could not fetch historical data'}), 404

if __name__ == '__main__':
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    print("\n" + "="*60)
    print("  🚀 StockSense - Live Stock Prices")
    print("="*60)
    print(f"🌐 Server running at: http://localhost:8080")
    print("="*60)
    print("\n💡 Features:")
    print("   - Live stock prices with real-time updates")
    print("   - Interactive stock graphs (Robinhood-style)")
    print("   - Price change indicators (green ↑ / red ↓)")
    print("   - Multiple time periods (1D, 5D, 1M, 3M, 6M, 1Y, 5Y)")
    print("\n⚠️  Press Ctrl+C to stop the server\n")
    print("="*60 + "\n")
    
    app.run(debug=True, host='127.0.0.1', port=8080)
