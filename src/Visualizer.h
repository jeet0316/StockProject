#pragma once
#include "StockData.h"
#include "DataProcessor.h"
#include <vector>
#include <string>

class Visualizer {
public:
    // Initialize visualizer (set up output directory, etc.)
    Visualizer(const std::string& outputDir = "output");
    
    // Plot price trends (line graph)
    bool PlotPriceTrend(const std::vector<StockData>& data, const std::string& ticker, 
                       const std::string& filename = "");
    
    // Plot with moving averages
    bool PlotWithMovingAverages(const std::vector<StockData>& data, 
                                const std::vector<double>& sma,
                                const std::vector<double>& ema,
                                const std::string& ticker,
                                const std::string& filename = "");
    
    // Plot volatility
    bool PlotVolatility(const std::vector<StockData>& data,
                       const std::vector<double>& volatility,
                       const std::string& ticker,
                       const std::string& filename = "");
    
    // Plot RSI
    bool PlotRSI(const std::vector<StockData>& data,
                const std::vector<double>& rsi,
                const std::string& ticker,
                const std::string& filename = "");
    
    // Plot MACD
    bool PlotMACD(const std::vector<StockData>& data,
                 const DataProcessor::MACDResult& macd,
                 const std::string& ticker,
                 const std::string& filename = "");
    
    // Plot Bollinger Bands
    bool PlotBollingerBands(const std::vector<StockData>& data,
                           const DataProcessor::BollingerBands& bands,
                           const std::string& ticker,
                           const std::string& filename = "");
    
    // Plot multiple stocks comparison
    bool PlotMultipleStocks(const std::vector<std::vector<StockData>>& stocksData,
                           const std::vector<std::string>& tickers,
                           const std::string& filename = "");
    
    // Generate console summary report
    void PrintConsoleSummary(const std::vector<StockData>& data, 
                            const std::string& ticker);
    
    // Generate HTML report (for web visualization)
    bool GenerateHTMLReport(const std::vector<StockData>& data,
                           const std::vector<double>& sma,
                           const std::vector<double>& volatility,
                           const std::string& ticker,
                           const std::string& filename = "");

private:
    std::string outputDirectory;
    
    // Helper to generate data file for gnuplot or matplotlib-cpp
    bool SaveDataForPlotting(const std::vector<StockData>& data, 
                            const std::string& dataFile);
    
    // Generate plot using gnuplot (if available)
    bool PlotWithGnuplot(const std::string& dataFile, 
                        const std::string& outputFile,
                        const std::string& title);
    
    // Generate plot using matplotlib-cpp (if available)
    bool PlotWithMatplotlib(const std::vector<StockData>& data,
                           const std::string& outputFile,
                           const std::string& title);
};

