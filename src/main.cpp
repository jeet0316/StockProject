#include "StockDataLoader.h"
#include "DataProcessor.h"
#include "Visualizer.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>

void PrintMenu() {
    std::cout << "\n========================================\n";
    std::cout << "     StockSense - Stock Analysis Tool\n";
    std::cout << "========================================\n";
    std::cout << "1. Analyze Single Stock\n";
    std::cout << "2. Compare Multiple Stocks\n";
    std::cout << "3. Perform PCA Analysis\n";
    std::cout << "4. Load from CSV File\n";
    std::cout << "5. Generate All Reports\n";
    std::cout << "6. Exit\n";
    std::cout << "========================================\n";
    std::cout << "Enter your choice: ";
}

void AnalyzeSingleStock(StockDataLoader& loader, DataProcessor& processor, Visualizer& visualizer) {
    std::string ticker;
    std::string startDate, endDate;
    
    std::cout << "\n--- Single Stock Analysis ---\n";
    std::cout << "Enter stock ticker (e.g., AAPL): ";
    std::cin >> ticker;
    
    std::cout << "Enter start date (YYYY-MM-DD): ";
    std::cin >> startDate;
    
    std::cout << "Enter end date (YYYY-MM-DD): ";
    std::cin >> endDate;
    
    std::cout << "\nFetching data for " << ticker << "...\n";
    auto data = loader.LoadFromAPI(ticker, startDate, endDate);
    
    if (data.empty()) {
        std::cout << "API fetch failed. Trying CSV file...\n";
        data = loader.LoadFromCSV(ticker + ".csv");
    }
    
    if (data.empty()) {
        std::cerr << "Error: Could not load data for " << ticker << "\n";
        return;
    }
    
    std::cout << "Loaded " << data.size() << " data points.\n";
    
    // Calculate all indicators
    std::cout << "\nCalculating indicators...\n";
    auto sma20 = processor.CalculateSMA(data, 20);
    auto sma50 = processor.CalculateSMA(data, 50);
    auto ema20 = processor.CalculateEMA(data, 20);
    auto volatility = processor.CalculateRollingVolatility(data, 20);
    auto rsi = processor.CalculateRSI(data, 14);
    auto macd = processor.CalculateMACD(data);
    auto bollinger = processor.CalculateBollingerBands(data, 20, 2.0);
    auto returns = processor.CalculateReturns(data);
    
    // Print summary
    visualizer.PrintConsoleSummary(data, ticker);
    
    // Print statistics
    std::cout << "\n--- Statistical Summary ---\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Mean Return: " << processor.CalculateMean(returns) * 100 << "%\n";
    std::cout << "Std Dev Returns: " << processor.CalculateStdDev(returns) * 100 << "%\n";
    if (!volatility.empty()) {
        std::cout << "Average Volatility: " << processor.CalculateMean(volatility) * 100 << "%\n";
    }
    if (!rsi.empty()) {
        std::cout << "Current RSI: " << rsi.back() << "\n";
        if (rsi.back() > 70) std::cout << "  -> Overbought\n";
        else if (rsi.back() < 30) std::cout << "  -> Oversold\n";
    }
    
    // Generate visualizations
    std::cout << "\nGenerating visualizations...\n";
    visualizer.PlotPriceTrend(data, ticker);
    visualizer.PlotWithMovingAverages(data, sma20, ema20, ticker);
    visualizer.PlotVolatility(data, volatility, ticker);
    visualizer.PlotRSI(data, rsi, ticker);
    visualizer.PlotMACD(data, macd, ticker);
    visualizer.PlotBollingerBands(data, bollinger, ticker);
    
    // Generate HTML report
    visualizer.GenerateHTMLReport(data, sma20, volatility, ticker);
    
    std::cout << "\nAnalysis complete! Check the 'output' directory for results.\n";
}

void CompareMultipleStocks(StockDataLoader& loader, DataProcessor& processor, Visualizer& visualizer) {
    int numStocks;
    std::cout << "\n--- Multiple Stock Comparison ---\n";
    std::cout << "How many stocks to compare? ";
    std::cin >> numStocks;
    
    if (numStocks < 2) {
        std::cerr << "Error: Need at least 2 stocks for comparison\n";
        return;
    }
    
    std::vector<std::string> tickers;
    std::vector<std::vector<StockData>> stocksData;
    std::string startDate, endDate;
    
    std::cout << "Enter start date (YYYY-MM-DD): ";
    std::cin >> startDate;
    std::cout << "Enter end date (YYYY-MM-DD): ";
    std::cin >> endDate;
    
    for (int i = 0; i < numStocks; ++i) {
        std::string ticker;
        std::cout << "Enter ticker #" << (i+1) << ": ";
        std::cin >> ticker;
        tickers.push_back(ticker);
        
        std::cout << "Fetching " << ticker << "...\n";
        auto data = loader.LoadFromAPI(ticker, startDate, endDate);
        if (data.empty()) {
            data = loader.LoadFromCSV(ticker + ".csv");
        }
        
        if (!data.empty()) {
            stocksData.push_back(data);
            std::cout << "  Loaded " << data.size() << " points\n";
        } else {
            std::cerr << "  Warning: Could not load " << ticker << "\n";
        }
    }
    
    if (stocksData.size() < 2) {
        std::cerr << "Error: Need at least 2 stocks with valid data\n";
        return;
    }
    
    // Calculate correlations
    std::cout << "\n--- Correlation Matrix ---\n";
    std::vector<std::vector<double>> returns;
    for (const auto& stock : stocksData) {
        returns.push_back(processor.CalculateReturns(stock));
    }
    
    // Align returns to same length
    size_t minSize = SIZE_MAX;
    for (auto& ret : returns) {
        minSize = std::min(minSize, ret.size());
    }
    for (auto& ret : returns) {
        if (ret.size() > minSize) {
            ret.erase(ret.begin(), ret.begin() + (ret.size() - minSize));
        }
    }
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "      ";
    for (size_t i = 0; i < tickers.size() && i < stocksData.size(); ++i) {
        std::cout << std::setw(8) << tickers[i];
    }
    std::cout << "\n";
    
    for (size_t i = 0; i < returns.size(); ++i) {
        std::cout << std::setw(6) << tickers[i];
        for (size_t j = 0; j < returns.size(); ++j) {
            double corr = processor.CalculateCorrelation(returns[i], returns[j]);
            std::cout << std::setw(8) << corr;
        }
        std::cout << "\n";
    }
    
    // Plot comparison
    visualizer.PlotMultipleStocks(stocksData, tickers);
    std::cout << "\nComparison complete! Check output directory.\n";
}

void PerformPCA(StockDataLoader& loader, DataProcessor& processor, Visualizer& visualizer) {
    int numStocks;
    int topN;
    
    std::cout << "\n--- PCA Analysis ---\n";
    std::cout << "How many stocks to analyze? ";
    std::cin >> numStocks;
    std::cout << "How many top influential stocks to find? ";
    std::cin >> topN;
    
    std::vector<std::string> tickers;
    std::vector<std::vector<StockData>> stocksData;
    std::string startDate, endDate;
    
    std::cout << "Enter start date (YYYY-MM-DD): ";
    std::cin >> startDate;
    std::cout << "Enter end date (YYYY-MM-DD): ";
    std::cin >> endDate;
    
    for (int i = 0; i < numStocks; ++i) {
        std::string ticker;
        std::cout << "Enter ticker #" << (i+1) << ": ";
        std::cin >> ticker;
        tickers.push_back(ticker);
        
        std::cout << "Fetching " << ticker << "...\n";
        auto data = loader.LoadFromAPI(ticker, startDate, endDate);
        if (data.empty()) {
            data = loader.LoadFromCSV(ticker + ".csv");
        }
        
        if (!data.empty()) {
            stocksData.push_back(data);
            std::cout << "  Loaded " << data.size() << " points\n";
        }
    }
    
    if (stocksData.size() < 2) {
        std::cerr << "Error: Need at least 2 stocks for PCA\n";
        return;
    }
    
    std::cout << "\nPerforming PCA analysis...\n";
    auto pcaResult = processor.PerformPCA(stocksData, tickers, topN);
    
    if (pcaResult.success) {
        std::cout << "\n--- PCA Results ---\n";
        std::cout << "Top " << topN << " Influential Stocks:\n";
        for (size_t i = 0; i < pcaResult.influentialStocks.size(); ++i) {
            std::cout << (i+1) << ". " << pcaResult.influentialStocks[i];
            if (i < pcaResult.explainedVariance.size()) {
                std::cout << " (Variance: " << std::fixed << std::setprecision(6) 
                         << pcaResult.explainedVariance[i] << ")";
            }
            std::cout << "\n";
        }
    } else {
        std::cerr << "PCA analysis failed.\n";
    }
}

void LoadFromCSV(StockDataLoader& loader, DataProcessor& processor, Visualizer& visualizer) {
    std::string filepath;
    std::cout << "\n--- Load from CSV ---\n";
    std::cout << "Enter CSV file path: ";
    std::cin >> filepath;
    
    std::cout << "Loading data from " << filepath << "...\n";
    auto data = loader.LoadFromCSV(filepath);
    
    if (data.empty()) {
        std::cerr << "Error: Could not load data from " << filepath << "\n";
        return;
    }
    
    std::cout << "Loaded " << data.size() << " data points.\n";
    
    // Extract ticker from filename
    std::string ticker = filepath;
    size_t lastSlash = ticker.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        ticker = ticker.substr(lastSlash + 1);
    }
    size_t lastDot = ticker.find_last_of(".");
    if (lastDot != std::string::npos) {
        ticker = ticker.substr(0, lastDot);
    }
    
    visualizer.PrintConsoleSummary(data, ticker);
    
    auto sma20 = processor.CalculateSMA(data, 20);
    auto volatility = processor.CalculateRollingVolatility(data, 20);
    
    visualizer.PlotPriceTrend(data, ticker);
    visualizer.GenerateHTMLReport(data, sma20, volatility, ticker);
    
    std::cout << "Analysis complete!\n";
}

void GenerateAllReports(StockDataLoader& loader, DataProcessor& processor, Visualizer& visualizer) {
    std::string ticker;
    std::string startDate, endDate;
    
    std::cout << "\n--- Generate All Reports ---\n";
    std::cout << "Enter stock ticker: ";
    std::cin >> ticker;
    std::cout << "Enter start date (YYYY-MM-DD): ";
    std::cin >> startDate;
    std::cout << "Enter end date (YYYY-MM-DD): ";
    std::cin >> endDate;
    
    std::cout << "\nFetching data...\n";
    auto data = loader.LoadFromAPI(ticker, startDate, endDate);
    if (data.empty()) {
        data = loader.LoadFromCSV(ticker + ".csv");
    }
    
    if (data.empty()) {
        std::cerr << "Error: Could not load data\n";
        return;
    }
    
    std::cout << "Calculating all indicators...\n";
    auto sma20 = processor.CalculateSMA(data, 20);
    auto sma50 = processor.CalculateSMA(data, 50);
    auto ema20 = processor.CalculateEMA(data, 20);
    auto volatility = processor.CalculateRollingVolatility(data, 20);
    auto rsi = processor.CalculateRSI(data, 14);
    auto macd = processor.CalculateMACD(data);
    auto bollinger = processor.CalculateBollingerBands(data, 20, 2.0);
    
    std::cout << "Generating all visualizations...\n";
    visualizer.PlotPriceTrend(data, ticker);
    visualizer.PlotWithMovingAverages(data, sma20, ema20, ticker);
    visualizer.PlotVolatility(data, volatility, ticker);
    visualizer.PlotRSI(data, rsi, ticker);
    visualizer.PlotMACD(data, macd, ticker);
    visualizer.PlotBollingerBands(data, bollinger, ticker);
    visualizer.GenerateHTMLReport(data, sma20, volatility, ticker);
    
    visualizer.PrintConsoleSummary(data, ticker);
    
    std::cout << "\nAll reports generated! Check the 'output' directory.\n";
}

int main() {
    StockDataLoader loader;
    DataProcessor processor;
    Visualizer visualizer("output");
    
    int choice;
    bool running = true;
    
    std::cout << "Welcome to StockSense!\n";
    std::cout << "Stock Market Analysis and Visualization Tool\n";
    
    while (running) {
        PrintMenu();
        std::cin >> choice;
        
        // Clear input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            case 1:
                AnalyzeSingleStock(loader, processor, visualizer);
                break;
            case 2:
                CompareMultipleStocks(loader, processor, visualizer);
                break;
            case 3:
                PerformPCA(loader, processor, visualizer);
                break;
            case 4:
                LoadFromCSV(loader, processor, visualizer);
                break;
            case 5:
                GenerateAllReports(loader, processor, visualizer);
                break;
            case 6:
                std::cout << "\nThank you for using StockSense!\n";
                running = false;
                break;
            default:
                std::cout << "\nInvalid choice. Please try again.\n";
                break;
        }
        
        if (running) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
    }
    
    return 0;
}
