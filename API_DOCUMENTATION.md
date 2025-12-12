# API Documentation - StockSense

## Stock Market API Used

### **Yahoo Finance API** (Free, No API Key Required)

We use Yahoo Finance's public API endpoints to fetch stock data. This is a free, unofficial API that doesn't require registration or API keys.

---

## API Endpoints Used

### 1. **Live Stock Quote API**
**Endpoint:** `https://query1.finance.yahoo.com/v8/finance/chart/{TICKER}`

**Used for:** Getting current stock price, open, high, low, volume, and previous close

**Example:**
```
GET https://query1.finance.yahoo.com/v8/finance/chart/AAPL?interval=1d&range=5d
```

**Parameters:**
- `{TICKER}` - Stock symbol (e.g., AAPL, GOOGL, MSFT)
- `interval=1d` - Data interval (1 day)
- `range=5d` - Time range (5 days to get complete data)

**Response Format:** JSON
```json
{
  "chart": {
    "result": [{
      "meta": {
        "regularMarketPrice": 277.82,
        "previousClose": 277.89,
        "open": 278.16,
        "high": 288.62,
        "low": 276.15,
        "volume": 13647895
      },
      "indicators": {
        "quote": [{
          "open": [278.16, ...],
          "high": [288.62, ...],
          "low": [276.15, ...],
          "close": [277.82, ...],
          "volume": [13647895, ...]
        }]
      },
      "timestamp": [1704067200, ...]
    }]
  }
}
```

**Location in Code:**
- `scripts/app.py` - `get_live_quote()` function (line 18-99)

---

### 2. **Historical Stock Data API**
**Endpoint:** `https://query1.finance.yahoo.com/v8/finance/chart/{TICKER}`

**Used for:** Getting historical price data for charts (graphs)

**Example:**
```
GET https://query1.finance.yahoo.com/v8/finance/chart/AAPL?interval=1d&range=1mo
```

**Parameters:**
- `{TICKER}` - Stock symbol
- `interval` - Data interval:
  - `5m` - 5 minutes (for 1D)
  - `15m` - 15 minutes (for 5D)
  - `1d` - 1 day (for 1M, 3M, 6M, 1Y)
  - `1wk` - 1 week (for 5Y)
- `range` - Time range:
  - `1d` - 1 day
  - `5d` - 5 days
  - `1mo` - 1 month
  - `3mo` - 3 months
  - `6mo` - 6 months
  - `1y` - 1 year
  - `5y` - 5 years

**Response Format:** JSON (same structure as quote API)

**Location in Code:**
- `scripts/app.py` - `get_historical_data()` function (line 101-150)

---

### 3. **Yahoo Finance CSV Download API** (C++ Code)
**Endpoint:** `https://query1.finance.yahoo.com/v7/finance/download/{TICKER}`

**Used for:** Downloading historical data as CSV (used in C++ code)

**Example:**
```
GET https://query1.finance.yahoo.com/v7/finance/download/AAPL?period1=1704067200&period2=1735689600&interval=1d&events=history
```

**Parameters:**
- `{TICKER}` - Stock symbol
- `period1` - Start date (Unix timestamp)
- `period2` - End date (Unix timestamp)
- `interval=1d` - Daily data
- `events=history` - Historical data

**Location in Code:**
- `src/StockDataLoader.cpp` - `LoadFromAPI()` function (line 109-150)

---

## Important Notes

### ⚠️ User-Agent Header Required
Yahoo Finance blocks requests without a proper User-Agent header. We include:
```python
headers = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
}
```

### 📊 API Limitations
- **Rate Limits:** Yahoo Finance may throttle requests if too many are made
- **Unofficial API:** This is an unofficial endpoint, so it may change without notice
- **No Guarantees:** No SLA or official support from Yahoo

### 🔄 Data Updates
- **Live Prices:** Updates during market hours (9:30 AM - 4:00 PM EST)
- **After Hours:** Shows last closing price
- **Weekends/Holidays:** Shows last trading day's data

---

## Alternative APIs (If Yahoo Finance Fails)

If you need a more reliable API, consider:

1. **Alpha Vantage** - Free tier: 5 API calls/minute, 500 calls/day
   - Requires API key (free registration)
   - URL: `https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol={TICKER}&apikey={API_KEY}`

2. **IEX Cloud** - Free tier available
   - Requires API key
   - URL: `https://cloud.iexapis.com/stable/stock/{TICKER}/quote?token={API_KEY}`

3. **Finnhub** - Free tier: 60 API calls/minute
   - Requires API key
   - URL: `https://finnhub.io/api/v1/quote?symbol={TICKER}&token={API_KEY}`

---

## Code References

### Python (Flask App)
- **File:** `scripts/app.py`
- **Functions:**
  - `get_live_quote()` - Fetches current stock quote
  - `get_historical_data()` - Fetches historical data for charts

### C++ Code
- **File:** `src/StockDataLoader.cpp`
- **Functions:**
  - `LoadFromAPI()` - Downloads CSV data from Yahoo Finance
  - `GetLatestQuote()` - Gets live quote (uses v8 API)
  - `GetRecentData()` - Gets recent historical data

---

## Example API Calls

### Get Live Quote for AAPL:
```bash
curl -H "User-Agent: Mozilla/5.0" \
  "https://query1.finance.yahoo.com/v8/finance/chart/AAPL?interval=1d&range=5d"
```

### Get 1 Month Historical Data:
```bash
curl -H "User-Agent: Mozilla/5.0" \
  "https://query1.finance.yahoo.com/v8/finance/chart/AAPL?interval=1d&range=1mo"
```

---

## References

- Yahoo Finance: https://finance.yahoo.com
- API Endpoint: `query1.finance.yahoo.com/v8/finance/chart/`
- No official documentation (unofficial API)

