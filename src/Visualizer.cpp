#include "Visualizer.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#if __cplusplus >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

Visualizer::Visualizer(const std::string& outputDir) : outputDirectory(outputDir) {
    // Create output directory if it doesn't exist
#if __cplusplus >= 201703L
    fs::create_directories(outputDir);
#else
    // Fallback for older C++ standards
    struct stat info;
    if (stat(outputDir.c_str(), &info) != 0) {
        mkdir(outputDir.c_str(), 0755);
    }
#endif
}

bool Visualizer::SaveDataForPlotting(const std::vector<StockData>& data, 
                                     const std::string& dataFile) {
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << dataFile << std::endl;
        return false;
    }
    
    file << "Date,Open,High,Low,Close,Volume\n";
    for (const auto& d : data) {
        file << d.date << "," << d.open << "," << d.high << "," 
             << d.low << "," << d.close << "," << d.volume << "\n";
    }
    file.close();
    return true;
}

bool Visualizer::PlotPriceTrend(const std::vector<StockData>& data, 
                                const std::string& ticker,
                                const std::string& filename) {
    std::string outputFile = filename.empty() 
        ? outputDirectory + "/" + ticker + "_price_trend.png" 
        : filename;
    
    // For now, save data to CSV for external plotting
    // TODO: Integrate matplotlib-cpp or gnuplot for direct plotting
    std::string dataFile = outputDirectory + "/" + ticker + "_plot_data.csv";
    if (!SaveDataForPlotting(data, dataFile)) {
        return false;
    }
    
    std::cout << "Data saved to " << dataFile << " for plotting\n";
    std::cout << "To plot, use: python scripts/plot_data.py " << dataFile << " " << outputFile << "\n";
    
    return true;
}

bool Visualizer::PlotWithMovingAverages(const std::vector<StockData>& data,
                                       const std::vector<double>& sma,
                                       const std::vector<double>& ema,
                                       const std::string& ticker,
                                       const std::string& filename) {
    std::string outputFile = filename.empty()
        ? outputDirectory + "/" + ticker + "_with_ma.png"
        : filename;
    
    // Save data with moving averages
    std::string dataFile = outputDirectory + "/" + ticker + "_ma_data.csv";
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Date,Close,SMA,EMA\n";
    size_t maOffset = data.size() - sma.size();
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i].date << "," << data[i].close;
        if (i >= maOffset && (i - maOffset) < sma.size()) {
            size_t maIdx = i - maOffset;
            file << "," << sma[maIdx];
            if (maIdx < ema.size()) {
                file << "," << ema[maIdx];
            } else {
                file << ",";
            }
        } else {
            file << ",,";
        }
        file << "\n";
    }
    file.close();
    
    std::cout << "Moving average data saved to " << dataFile << "\n";
    return true;
}

bool Visualizer::PlotVolatility(const std::vector<StockData>& data,
                               const std::vector<double>& volatility,
                               const std::string& ticker,
                               const std::string& filename) {
    std::string outputFile = filename.empty()
        ? outputDirectory + "/" + ticker + "_volatility.png"
        : filename;
    
    std::string dataFile = outputDirectory + "/" + ticker + "_volatility_data.csv";
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Date,Volatility\n";
    size_t volOffset = data.size() - volatility.size();
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i].date;
        if (i >= volOffset && (i - volOffset) < volatility.size()) {
            file << "," << volatility[i - volOffset];
        } else {
            file << ",";
        }
        file << "\n";
    }
    file.close();
    
    std::cout << "Volatility data saved to " << dataFile << "\n";
    return true;
}

bool Visualizer::PlotRSI(const std::vector<StockData>& data,
                        const std::vector<double>& rsi,
                        const std::string& ticker,
                        const std::string& filename) {
    std::string outputFile = filename.empty()
        ? outputDirectory + "/" + ticker + "_rsi.png"
        : filename;
    
    std::string dataFile = outputDirectory + "/" + ticker + "_rsi_data.csv";
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Date,RSI\n";
    size_t rsiOffset = data.size() - rsi.size();
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i].date;
        if (i >= rsiOffset && (i - rsiOffset) < rsi.size()) {
            file << "," << rsi[i - rsiOffset];
        } else {
            file << ",";
        }
        file << "\n";
    }
    file.close();
    
    std::cout << "RSI data saved to " << dataFile << "\n";
    return true;
}

bool Visualizer::PlotMACD(const std::vector<StockData>& data,
                         const DataProcessor::MACDResult& macd,
                         const std::string& ticker,
                         const std::string& filename) {
    std::string outputFile = filename.empty()
        ? outputDirectory + "/" + ticker + "_macd.png"
        : filename;
    
    std::string dataFile = outputDirectory + "/" + ticker + "_macd_data.csv";
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Date,MACD,Signal,Histogram\n";
    size_t macdOffset = data.size() - macd.macd.size();
    size_t signalOffset = macd.macd.size() - macd.signal.size();
    
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i].date;
        
        if (i >= macdOffset && (i - macdOffset) < macd.macd.size()) {
            size_t macdIdx = i - macdOffset;
            file << "," << macd.macd[macdIdx];
            
            if (macdIdx >= signalOffset && (macdIdx - signalOffset) < macd.signal.size()) {
                size_t signalIdx = macdIdx - signalOffset;
                file << "," << macd.signal[signalIdx];
                if (signalIdx < macd.histogram.size()) {
                    file << "," << macd.histogram[signalIdx];
                } else {
                    file << ",";
                }
            } else {
                file << ",,";
            }
        } else {
            file << ",,,";
        }
        file << "\n";
    }
    file.close();
    
    std::cout << "MACD data saved to " << dataFile << "\n";
    return true;
}

bool Visualizer::PlotBollingerBands(const std::vector<StockData>& data,
                                   const DataProcessor::BollingerBands& bands,
                                   const std::string& ticker,
                                   const std::string& filename) {
    std::string outputFile = filename.empty()
        ? outputDirectory + "/" + ticker + "_bollinger.png"
        : filename;
    
    std::string dataFile = outputDirectory + "/" + ticker + "_bollinger_data.csv";
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Date,Close,Upper,Middle,Lower\n";
    size_t bandsOffset = data.size() - bands.upper.size();
    
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i].date << "," << data[i].close;
        if (i >= bandsOffset && (i - bandsOffset) < bands.upper.size()) {
            size_t bandsIdx = i - bandsOffset;
            file << "," << bands.upper[bandsIdx] 
                 << "," << bands.middle[bandsIdx]
                 << "," << bands.lower[bandsIdx];
        } else {
            file << ",,,";
        }
        file << "\n";
    }
    file.close();
    
    std::cout << "Bollinger Bands data saved to " << dataFile << "\n";
    return true;
}

bool Visualizer::PlotMultipleStocks(const std::vector<std::vector<StockData>>& stocksData,
                                    const std::vector<std::string>& tickers,
                                    const std::string& filename) {
    if (stocksData.size() != tickers.size()) {
        std::cerr << "Error: Mismatch between stocks data and tickers\n";
        return false;
    }
    
    std::string outputFile = filename.empty()
        ? outputDirectory + "/multiple_stocks_comparison.png"
        : filename;
    
    // Save combined data
    std::string dataFile = outputDirectory + "/multiple_stocks_data.csv";
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Date";
    for (const auto& ticker : tickers) {
        file << "," << ticker;
    }
    file << "\n";
    
    // Find max size
    size_t maxSize = 0;
    for (const auto& stock : stocksData) {
        maxSize = std::max(maxSize, stock.size());
    }
    
    for (size_t i = 0; i < maxSize; ++i) {
        if (i < stocksData[0].size()) {
            file << stocksData[0][i].date;
        } else {
            file << ",";
        }
        
        for (size_t j = 0; j < stocksData.size(); ++j) {
            file << ",";
            if (i < stocksData[j].size()) {
                file << stocksData[j][i].close;
            }
        }
        file << "\n";
    }
    file.close();
    
    std::cout << "Multiple stocks data saved to " << dataFile << "\n";
    return true;
}

void Visualizer::PrintConsoleSummary(const std::vector<StockData>& data, 
                                     const std::string& ticker) {
    if (data.empty()) {
        std::cout << "No data available for " << ticker << "\n";
        return;
    }
    
    std::cout << "\n=== Stock Summary: " << ticker << " ===\n";
    std::cout << "Date Range: " << data.front().date << " to " << data.back().date << "\n";
    std::cout << "Total Days: " << data.size() << "\n";
    
    double minPrice = data[0].low;
    double maxPrice = data[0].high;
    double totalVolume = 0.0;
    
    for (const auto& d : data) {
        minPrice = std::min(minPrice, d.low);
        maxPrice = std::max(maxPrice, d.high);
        totalVolume += d.volume;
    }
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Lowest Price: $" << minPrice << "\n";
    std::cout << "Highest Price: $" << maxPrice << "\n";
    std::cout << "Current Close: $" << data.back().close << "\n";
    std::cout << "Average Volume: " << std::setprecision(0) 
              << totalVolume / data.size() << "\n";
    std::cout << "========================\n\n";
}

bool Visualizer::GenerateHTMLReport(const std::vector<StockData>& data,
                                   const std::vector<double>& sma,
                                   const std::vector<double>& volatility,
                                   const std::string& ticker,
                                   const std::string& filename) {
    std::string outputFile = filename.empty()
        ? outputDirectory + "/" + ticker + "_report.html"
        : filename;
    
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "<!DOCTYPE html>\n<html><head><title>StockSense Report: " << ticker << "</title>\n";
    file << "<style>body{font-family:Arial;margin:20px;}table{border-collapse:collapse;width:100%;}";
    file << "th,td{border:1px solid #ddd;padding:8px;text-align:left;}th{background-color:#4CAF50;color:white;}</style>\n";
    file << "</head><body>\n";
    file << "<h1>StockSense Report: " << ticker << "</h1>\n";
    file << "<h2>Summary</h2>\n";
    file << "<p>Date Range: " << data.front().date << " to " << data.back().date << "</p>\n";
    file << "<p>Total Data Points: " << data.size() << "</p>\n";
    
    file << "<h2>Recent Data</h2>\n";
    file << "<table><tr><th>Date</th><th>Open</th><th>High</th><th>Low</th><th>Close</th><th>Volume</th></tr>\n";
    
    size_t rowsToShow = std::min(static_cast<size_t>(20), data.size());
    for (size_t i = data.size() - rowsToShow; i < data.size(); ++i) {
        file << "<tr><td>" << data[i].date << "</td><td>$" << data[i].open 
             << "</td><td>$" << data[i].high << "</td><td>$" << data[i].low 
             << "</td><td>$" << data[i].close << "</td><td>" << data[i].volume << "</td></tr>\n";
    }
    file << "</table>\n";
    file << "</body></html>\n";
    file.close();
    
    std::cout << "HTML report generated: " << outputFile << "\n";
    return true;
}

bool Visualizer::PlotWithGnuplot(const std::string& dataFile,
                                 const std::string& outputFile,
                                 const std::string& title) {
    // TODO: Implement gnuplot integration
    std::cerr << "Gnuplot integration not yet implemented\n";
    return false;
}

bool Visualizer::PlotWithMatplotlib(const std::vector<StockData>& data,
                                   const std::string& outputFile,
                                   const std::string& title) {
    // TODO: Implement matplotlib-cpp integration
    std::cerr << "Matplotlib-cpp integration not yet implemented\n";
    return false;
}

