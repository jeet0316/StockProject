#pragma once
#include <string>
#include <vector>
#include <map>

class Config {
public:
    static Config& GetInstance();
    
    bool LoadFromFile(const std::string& filepath = "config.json");
    
    // Getters
    std::string GetDefaultTicker() const { return defaultTicker; }
    std::string GetDefaultStartDate() const { return defaultStartDate; }
    std::string GetDefaultEndDate() const { return defaultEndDate; }
    std::string GetOutputDirectory() const { return outputDirectory; }
    
    std::vector<int> GetSMAPeriods() const { return smaPeriods; }
    std::vector<int> GetEMAPeriods() const { return emaPeriods; }
    int GetRSIPeriod() const { return rsiPeriod; }
    int GetMACDFast() const { return macdFast; }
    int GetMACDSlow() const { return macdSlow; }
    int GetMACDSignal() const { return macdSignal; }
    int GetBollingerPeriod() const { return bollingerPeriod; }
    double GetBollingerStd() const { return bollingerStd; }
    int GetVolatilityWindow() const { return volatilityWindow; }
    
    int GetAPITimeout() const { return apiTimeout; }
    int GetAPIRetryCount() const { return apiRetryCount; }
    
    bool UsePython() const { return usePython; }
    std::string GetPythonScript() const { return pythonScript; }
    std::string GetImageFormat() const { return imageFormat; }
    int GetImageDPI() const { return imageDPI; }
    
private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    // Default values
    std::string defaultTicker = "AAPL";
    std::string defaultStartDate = "2024-01-01";
    std::string defaultEndDate = "2024-12-31";
    std::string outputDirectory = "output";
    
    std::vector<int> smaPeriods = {20, 50, 200};
    std::vector<int> emaPeriods = {20, 50};
    int rsiPeriod = 14;
    int macdFast = 12;
    int macdSlow = 26;
    int macdSignal = 9;
    int bollingerPeriod = 20;
    double bollingerStd = 2.0;
    int volatilityWindow = 20;
    
    int apiTimeout = 30;
    int apiRetryCount = 3;
    
    bool usePython = true;
    std::string pythonScript = "scripts/plot_data.py";
    std::string imageFormat = "png";
    int imageDPI = 300;
};

