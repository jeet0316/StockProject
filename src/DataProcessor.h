#pragma once
#include "StockData.h"
#include <vector>
#include <string>

class DataProcessor {
public:
    // Moving Averages
    std::vector<double> CalculateSMA(const std::vector<StockData>& data, int period);
    std::vector<double> CalculateEMA(const std::vector<StockData>& data, int period);
    
    // Volatility Analysis
    std::vector<double> CalculateVolatility(const std::vector<StockData>& data, int window);
    std::vector<double> CalculateRollingVolatility(const std::vector<StockData>& data, int window);
    
    // Returns
    std::vector<double> CalculateReturns(const std::vector<StockData>& data);
    
    // Technical Indicators
    std::vector<double> CalculateRSI(const std::vector<StockData>& data, int period = 14);
    struct MACDResult {
        std::vector<double> macd;
        std::vector<double> signal;
        std::vector<double> histogram;
    };
    MACDResult CalculateMACD(const std::vector<StockData>& data, 
                            int fastPeriod = 12, int slowPeriod = 26, int signalPeriod = 9);
    
    struct BollingerBands {
        std::vector<double> upper;
        std::vector<double> middle;  // SMA
        std::vector<double> lower;
    };
    BollingerBands CalculateBollingerBands(const std::vector<StockData>& data, 
                                          int period = 20, double stdDevMultiplier = 2.0);
    
    // Principal Component Analysis (PCA) for influential stocks
    // Returns the top N influential stocks and their explained variance
    struct PCAResult {
        std::vector<std::string> influentialStocks;
        std::vector<double> explainedVariance;
        std::vector<std::vector<double>> principalComponents;
        bool success;
    };
    
    PCAResult PerformPCA(const std::vector<std::vector<StockData>>& multipleStocks, 
                        const std::vector<std::string>& tickers, int topN = 3);
    
    // Statistical measures
    double CalculateMean(const std::vector<double>& values);
    double CalculateStdDev(const std::vector<double>& values);
    double CalculateVariance(const std::vector<double>& values);
    double CalculateMax(const std::vector<double>& values);
    double CalculateMin(const std::vector<double>& values);
    double CalculateMedian(const std::vector<double>& values);
    
    // Correlation
    double CalculateCorrelation(const std::vector<double>& x, const std::vector<double>& y);
    
private:
    // Helper functions for PCA
    std::vector<std::vector<double>> ComputeCovarianceMatrix(
        const std::vector<std::vector<double>>& returnsMatrix);
    std::vector<double> ComputeEigenvalues(const std::vector<std::vector<double>>& matrix);
    std::vector<std::vector<double>> ComputeEigenvectors(const std::vector<std::vector<double>>& matrix);
};

