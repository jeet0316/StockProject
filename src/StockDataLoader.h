#pragma once
#include <string>
#include <vector>
#include "StockData.h"

struct LiveQuote {
    std::string ticker;
    double currentPrice;
    double previousClose;
    double change;
    double changePercent;
    double open;
    double high;
    double low;
    long volume;
    std::string lastUpdate;
};

class StockDataLoader {
public:
    // Load data from a local CSV file
    std::vector<StockData> LoadFromCSV(const std::string& filepath);

    // Fetch data from Yahoo Finance API
    std::vector<StockData> LoadFromAPI(const std::string& ticker, 
                                       const std::string& startDate, 
                                       const std::string& endDate);
    
    // Get latest/current stock price (real-time quote)
    LiveQuote GetLatestQuote(const std::string& ticker);
    
    // Get recent data (last N days) - useful for daily updates
    std::vector<StockData> GetRecentData(const std::string& ticker, int days = 30);

private:
    std::vector<StockData> ParseCSV(const std::string& csvContent);
    std::string FetchFromURL(const std::string& url);
    LiveQuote ParseQuoteJSON(const std::string& jsonContent, const std::string& ticker);
};
