#include "DataProcessor.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

std::vector<double> DataProcessor::CalculateSMA(const std::vector<StockData>& data, int period) {
    std::vector<double> sma;
    if (data.size() < static_cast<size_t>(period)) {
        return sma;
    }
    
    for (size_t i = period - 1; i < data.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < period; ++j) {
            sum += data[i - j].close;
        }
        sma.push_back(sum / period);
    }
    return sma;
}

std::vector<double> DataProcessor::CalculateEMA(const std::vector<StockData>& data, int period) {
    std::vector<double> ema;
    if (data.empty() || period <= 0) {
        return ema;
    }
    
    double multiplier = 2.0 / (period + 1.0);
    double currentEMA = data[0].close;
    ema.push_back(currentEMA);
    
    for (size_t i = 1; i < data.size(); ++i) {
        currentEMA = (data[i].close - currentEMA) * multiplier + currentEMA;
        ema.push_back(currentEMA);
    }
    return ema;
}

std::vector<double> DataProcessor::CalculateReturns(const std::vector<StockData>& data) {
    std::vector<double> returns;
    if (data.size() < 2) {
        return returns;
    }
    
    for (size_t i = 1; i < data.size(); ++i) {
        double ret = (data[i].close - data[i-1].close) / data[i-1].close;
        returns.push_back(ret);
    }
    return returns;
}

std::vector<double> DataProcessor::CalculateVolatility(const std::vector<StockData>& data, int window) {
    std::vector<double> volatility;
    auto returns = CalculateReturns(data);
    
    if (returns.size() < static_cast<size_t>(window)) {
        return volatility;
    }
    
    for (size_t i = window - 1; i < returns.size(); ++i) {
        std::vector<double> windowReturns(returns.begin() + i - window + 1, returns.begin() + i + 1);
        double stdDev = CalculateStdDev(windowReturns);
        volatility.push_back(stdDev * std::sqrt(252.0)); // Annualized volatility
    }
    return volatility;
}

std::vector<double> DataProcessor::CalculateRollingVolatility(const std::vector<StockData>& data, int window) {
    return CalculateVolatility(data, window);
}

double DataProcessor::CalculateMean(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double DataProcessor::CalculateVariance(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    double mean = CalculateMean(values);
    double variance = 0.0;
    for (double val : values) {
        variance += (val - mean) * (val - mean);
    }
    return variance / values.size();
}

double DataProcessor::CalculateStdDev(const std::vector<double>& values) {
    return std::sqrt(CalculateVariance(values));
}

double DataProcessor::CalculateMax(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    return *std::max_element(values.begin(), values.end());
}

double DataProcessor::CalculateMin(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    return *std::min_element(values.begin(), values.end());
}

double DataProcessor::CalculateMedian(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    size_t n = sorted.size();
    if (n % 2 == 0) {
        return (sorted[n/2 - 1] + sorted[n/2]) / 2.0;
    } else {
        return sorted[n/2];
    }
}

double DataProcessor::CalculateCorrelation(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2) return 0.0;
    
    double meanX = CalculateMean(x);
    double meanY = CalculateMean(y);
    
    double numerator = 0.0;
    double sumXSq = 0.0;
    double sumYSq = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        double dx = x[i] - meanX;
        double dy = y[i] - meanY;
        numerator += dx * dy;
        sumXSq += dx * dx;
        sumYSq += dy * dy;
    }
    
    double denominator = std::sqrt(sumXSq * sumYSq);
    if (denominator == 0.0) return 0.0;
    
    return numerator / denominator;
}

std::vector<double> DataProcessor::CalculateRSI(const std::vector<StockData>& data, int period) {
    std::vector<double> rsi;
    if (data.size() < static_cast<size_t>(period + 1)) {
        return rsi;
    }
    
    std::vector<double> gains;
    std::vector<double> losses;
    
    // Calculate price changes
    for (size_t i = 1; i < data.size(); ++i) {
        double change = data[i].close - data[i-1].close;
        gains.push_back(change > 0 ? change : 0.0);
        losses.push_back(change < 0 ? -change : 0.0);
    }
    
    // Calculate initial average gain and loss
    double avgGain = CalculateMean(std::vector<double>(gains.begin(), gains.begin() + period));
    double avgLoss = CalculateMean(std::vector<double>(losses.begin(), losses.begin() + period));
    
    // Calculate RSI for remaining periods
    for (size_t i = period; i < gains.size(); ++i) {
        // Use Wilder's smoothing method
        avgGain = (avgGain * (period - 1) + gains[i]) / period;
        avgLoss = (avgLoss * (period - 1) + losses[i]) / period;
        
        if (avgLoss == 0.0) {
            rsi.push_back(100.0);
        } else {
            double rs = avgGain / avgLoss;
            rsi.push_back(100.0 - (100.0 / (1.0 + rs)));
        }
    }
    
    return rsi;
}

DataProcessor::MACDResult DataProcessor::CalculateMACD(const std::vector<StockData>& data,
                                                       int fastPeriod, int slowPeriod, int signalPeriod) {
    MACDResult result;
    
    if (data.size() < static_cast<size_t>(slowPeriod + signalPeriod)) {
        return result;
    }
    
    // Calculate EMAs
    auto fastEMA = CalculateEMA(data, fastPeriod);
    auto slowEMA = CalculateEMA(data, slowPeriod);
    
    // Calculate MACD line (fast EMA - slow EMA)
    size_t minSize = std::min(fastEMA.size(), slowEMA.size());
    for (size_t i = 0; i < minSize; ++i) {
        result.macd.push_back(fastEMA[i] - slowEMA[i]);
    }
    
    // Calculate Signal line (EMA of MACD)
    if (result.macd.size() >= static_cast<size_t>(signalPeriod)) {
        // Convert MACD to StockData format for EMA calculation
        std::vector<StockData> macdData;
        size_t offset = data.size() - result.macd.size();
        for (size_t i = 0; i < result.macd.size(); ++i) {
            StockData sd = data[offset + i];
            sd.close = result.macd[i];
            macdData.push_back(sd);
        }
        result.signal = CalculateEMA(macdData, signalPeriod);
        
        // Calculate Histogram (MACD - Signal)
        size_t signalOffset = result.macd.size() - result.signal.size();
        for (size_t i = 0; i < result.signal.size(); ++i) {
            result.histogram.push_back(result.macd[signalOffset + i] - result.signal[i]);
        }
    }
    
    return result;
}

DataProcessor::BollingerBands DataProcessor::CalculateBollingerBands(
    const std::vector<StockData>& data, int period, double stdDevMultiplier) {
    
    BollingerBands bands;
    
    if (data.size() < static_cast<size_t>(period)) {
        return bands;
    }
    
    auto sma = CalculateSMA(data, period);
    bands.middle = sma;
    
    // Calculate standard deviation for each period
    for (size_t i = period - 1; i < data.size(); ++i) {
        std::vector<double> window;
        for (int j = 0; j < period; ++j) {
            window.push_back(data[i - j].close);
        }
        double stdDev = CalculateStdDev(window);
        size_t smaIdx = i - (period - 1);
        double middle = bands.middle[smaIdx];
        bands.upper.push_back(middle + (stdDevMultiplier * stdDev));
        bands.lower.push_back(middle - (stdDevMultiplier * stdDev));
    }
    
    return bands;
}

DataProcessor::PCAResult DataProcessor::PerformPCA(
    const std::vector<std::vector<StockData>>& multipleStocks,
    const std::vector<std::string>& tickers, int topN) {
    
    PCAResult result;
    result.success = false;
    
    if (multipleStocks.empty() || multipleStocks.size() != tickers.size()) {
        std::cerr << "Error: Mismatch between stocks data and tickers\n";
        return result;
    }
    
    // Convert stock data to returns matrix
    std::vector<std::vector<double>> returnsMatrix;
    size_t minSize = SIZE_MAX;
    
    for (const auto& stock : multipleStocks) {
        auto returns = CalculateReturns(stock);
        returnsMatrix.push_back(returns);
        minSize = std::min(minSize, returns.size());
    }
    
    if (minSize < 2 || returnsMatrix.size() < 2) {
        std::cerr << "Error: Insufficient data for PCA\n";
        return result;
    }
    
    // Align all returns to same length
    for (auto& returns : returnsMatrix) {
        if (returns.size() > minSize) {
            returns.erase(returns.begin(), returns.begin() + (returns.size() - minSize));
        }
    }
    
    // Compute covariance matrix
    auto covMatrix = ComputeCovarianceMatrix(returnsMatrix);
    
    if (covMatrix.empty()) {
        std::cerr << "Error: Failed to compute covariance matrix\n";
        return result;
    }
    
    // Simple PCA: Find principal components by variance
    // For full PCA, Eigen library would be better, but this gives a working implementation
    std::vector<double> variances;
    for (size_t i = 0; i < covMatrix.size(); ++i) {
        variances.push_back(covMatrix[i][i]); // Diagonal elements are variances
    }
    
    // Get top N stocks by variance (simplified approach)
    std::vector<std::pair<double, size_t>> indexedVariances;
    for (size_t i = 0; i < variances.size(); ++i) {
        indexedVariances.push_back({variances[i], i});
    }
    
    std::sort(indexedVariances.rbegin(), indexedVariances.rend());
    
    int n = std::min(topN, static_cast<int>(indexedVariances.size()));
    for (int i = 0; i < n; ++i) {
        size_t idx = indexedVariances[i].second;
        result.influentialStocks.push_back(tickers[idx]);
        result.explainedVariance.push_back(indexedVariances[i].first);
    }
    
    result.success = true;
    return result;
}

std::vector<std::vector<double>> DataProcessor::ComputeCovarianceMatrix(
    const std::vector<std::vector<double>>& returnsMatrix) {
    
    if (returnsMatrix.empty() || returnsMatrix[0].empty()) {
        return {};
    }
    
    size_t n = returnsMatrix.size();
    size_t m = returnsMatrix[0].size();
    
    // Verify all rows have same length
    for (const auto& row : returnsMatrix) {
        if (row.size() != m) {
            std::cerr << "Error: Inconsistent returns matrix dimensions\n";
            return {};
        }
    }
    
    std::vector<std::vector<double>> covMatrix(n, std::vector<double>(n, 0.0));
    
    // Calculate means for each stock
    std::vector<double> means(n);
    for (size_t i = 0; i < n; ++i) {
        means[i] = CalculateMean(returnsMatrix[i]);
    }
    
    // Compute covariance matrix
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < m; ++k) {
                sum += (returnsMatrix[i][k] - means[i]) * (returnsMatrix[j][k] - means[j]);
            }
            covMatrix[i][j] = sum / (m - 1); // Sample covariance
        }
    }
    
    return covMatrix;
}

std::vector<double> DataProcessor::ComputeEigenvalues(const std::vector<std::vector<double>>& matrix) {
    // Simplified eigenvalue computation for 2x2 matrices
    // For larger matrices, Eigen library should be used
    if (matrix.size() == 2 && matrix[0].size() == 2) {
        double a = matrix[0][0];
        double b = matrix[0][1];
        double c = matrix[1][0];
        double d = matrix[1][1];
        
        double trace = a + d;
        double det = a * d - b * c;
        
        double discriminant = trace * trace - 4 * det;
        if (discriminant < 0) {
            return {};
        }
        
        double sqrtDisc = std::sqrt(discriminant);
        return {(trace + sqrtDisc) / 2.0, (trace - sqrtDisc) / 2.0};
    }
    
    // For larger matrices, return diagonal as approximation
    std::vector<double> eigenvalues;
    for (size_t i = 0; i < matrix.size() && i < matrix[i].size(); ++i) {
        eigenvalues.push_back(matrix[i][i]);
    }
    return eigenvalues;
}

std::vector<std::vector<double>> DataProcessor::ComputeEigenvectors(const std::vector<std::vector<double>>& matrix) {
    // Placeholder - full implementation requires Eigen library
    std::vector<std::vector<double>> eigenvectors;
    return eigenvectors;
}

