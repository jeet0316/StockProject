#include "StockDataLoader.h"
#include "rapidcsv.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <iomanip>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::vector<StockData> StockDataLoader::LoadFromCSV(const std::string& filepath) {
    std::vector<StockData> data;
    
    if (filepath.empty()) {
        std::cerr << "Error: Empty filepath provided\n";
        return data;
    }
    
    try {
        rapidcsv::Document doc(filepath, rapidcsv::LabelParams(0, -1));

        auto dates = doc.GetColumn<std::string>("Date");
        auto opens = doc.GetColumn<double>("Open");
        auto highs = doc.GetColumn<double>("High");
        auto lows = doc.GetColumn<double>("Low");
        auto closes = doc.GetColumn<double>("Close");
        auto volumes = doc.GetColumn<double>("Volume");

        // Validate data consistency
        size_t expectedSize = dates.size();
        if (opens.size() != expectedSize || highs.size() != expectedSize ||
            lows.size() != expectedSize || closes.size() != expectedSize ||
            volumes.size() != expectedSize) {
            std::cerr << "Warning: Inconsistent column sizes in CSV\n";
        }

        size_t validRows = 0;
        for (size_t i = 0; i < dates.size(); ++i) {
            // Validate data
            if (highs[i] < lows[i]) {
                std::cerr << "Warning: Row " << i << " has High < Low, skipping\n";
                continue;
            }
            if (opens[i] <= 0 || closes[i] <= 0 || volumes[i] < 0) {
                std::cerr << "Warning: Row " << i << " has invalid values, skipping\n";
                continue;
            }
            
            data.push_back({ dates[i], opens[i], highs[i], lows[i], closes[i], volumes[i] });
            validRows++;
        }
        
        if (validRows == 0) {
            std::cerr << "Error: No valid rows found in CSV file\n";
        } else if (validRows < dates.size()) {
            std::cerr << "Warning: Only " << validRows << " of " << dates.size() 
                     << " rows were valid\n";
        }
    } 
    catch (const std::exception& e) {
        std::cerr << "Error loading CSV from " << filepath << ": " << e.what() << std::endl;
    }
    
    return data;
}

std::string StockDataLoader::FetchFromURL(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (!curl) {
        std::cerr << "Error: Failed to initialize CURL\n";
        return readBuffer;
    }
    
    if (url.empty()) {
        std::cerr << "Error: Empty URL provided\n";
        curl_easy_cleanup(curl);
        return readBuffer;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "CURL Error: " << curl_easy_strerror(res) << "\n";
        readBuffer.clear();
    } else {
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode != 200) {
            std::cerr << "HTTP Error: Received status code " << responseCode << "\n";
            readBuffer.clear();
        }
    }
    
    curl_easy_cleanup(curl);
    return readBuffer;
}

std::vector<StockData> StockDataLoader::LoadFromAPI(
    const std::string& ticker, const std::string& startDate, const std::string& endDate) {

    // Convert dates from YYYY-MM-DD to UNIX timestamps
    // Simple parser for YYYY-MM-DD format
    auto dateToTimestamp = [](const std::string& dateStr) -> long {
        // Format: YYYY-MM-DD
        if (dateStr.length() != 10) return 0;
        
        int year = std::stoi(dateStr.substr(0, 4));
        int month = std::stoi(dateStr.substr(5, 2));
        int day = std::stoi(dateStr.substr(8, 2));
        
        // Simple date to timestamp conversion (approximate)
        // For more accuracy, use a proper date library
        std::tm timeinfo = {};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        
        return std::mktime(&timeinfo);
    };
    
    long period1 = dateToTimestamp(startDate);
    long period2 = dateToTimestamp(endDate);
    
    if (period1 == 0 || period2 == 0) {
        std::cerr << "Warning: Invalid date format. Using default dates.\n";
        period1 = 1704067200; // 2024-01-01
        period2 = 1735689600; // 2025-01-01
    }
    
    std::string url = "https://query1.finance.yahoo.com/v7/finance/download/" + ticker +
                      "?period1=" + std::to_string(period1) + 
                      "&period2=" + std::to_string(period2) + 
                      "&interval=1d&events=history";
    std::string csvContent = FetchFromURL(url);
    return ParseCSV(csvContent);
}

std::vector<StockData> StockDataLoader::ParseCSV(const std::string& csvContent) {
    std::vector<StockData> data;
    std::stringstream ss(csvContent);
    std::string line;
    bool firstLine = true;

    while (std::getline(ss, line)) {
        if (firstLine) { firstLine = false; continue; }  // skip header

        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> tokens;

        while (std::getline(lineStream, cell, ',')) tokens.push_back(cell);
        if (tokens.size() < 6) continue;

        try {
            data.push_back({
                tokens[0],
                std::stod(tokens[1]),
                std::stod(tokens[2]),
                std::stod(tokens[3]),
                std::stod(tokens[4]),
                std::stod(tokens[6])
            });
        } catch (...) {
            continue; // skip bad rows
        }
    }
    return data;
}

LiveQuote StockDataLoader::GetLatestQuote(const std::string& ticker) {
    LiveQuote quote;
    quote.ticker = ticker;
    
    // Use Yahoo Finance quote API
    std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + ticker + 
                      "?interval=1d&range=1d";
    
    std::string jsonContent = FetchFromURL(url);
    
    if (jsonContent.empty()) {
        std::cerr << "Error: Could not fetch quote for " << ticker << "\n";
        return quote;
    }
    
    // Parse JSON response (simplified - for production use a JSON library)
    try {
        // Extract current price from JSON
        size_t pricePos = jsonContent.find("\"regularMarketPrice\":");
        if (pricePos != std::string::npos) {
            size_t start = jsonContent.find_first_of("0123456789.", pricePos);
            size_t end = jsonContent.find_first_not_of("0123456789.", start);
            if (start != std::string::npos && end != std::string::npos) {
                quote.currentPrice = std::stod(jsonContent.substr(start, end - start));
            }
        }
        
        // Extract previous close
        size_t prevClosePos = jsonContent.find("\"previousClose\":");
        if (prevClosePos != std::string::npos) {
            size_t start = jsonContent.find_first_of("0123456789.", prevClosePos);
            size_t end = jsonContent.find_first_not_of("0123456789.", start);
            if (start != std::string::npos && end != std::string::npos) {
                quote.previousClose = std::stod(jsonContent.substr(start, end - start));
            }
        }
        
        // Extract open
        size_t openPos = jsonContent.find("\"open\":");
        if (openPos != std::string::npos) {
            size_t start = jsonContent.find_first_of("0123456789.", openPos);
            size_t end = jsonContent.find_first_not_of("0123456789.", start);
            if (start != std::string::npos && end != std::string::npos) {
                quote.open = std::stod(jsonContent.substr(start, end - start));
            }
        }
        
        // Extract high
        size_t highPos = jsonContent.find("\"high\":");
        if (highPos != std::string::npos) {
            size_t start = jsonContent.find_first_of("0123456789.", highPos);
            size_t end = jsonContent.find_first_not_of("0123456789.", start);
            if (start != std::string::npos && end != std::string::npos) {
                quote.high = std::stod(jsonContent.substr(start, end - start));
            }
        }
        
        // Extract low
        size_t lowPos = jsonContent.find("\"low\":");
        if (lowPos != std::string::npos) {
            size_t start = jsonContent.find_first_of("0123456789.", lowPos);
            size_t end = jsonContent.find_first_not_of("0123456789.", start);
            if (start != std::string::npos && end != std::string::npos) {
                quote.low = std::stod(jsonContent.substr(start, end - start));
            }
        }
        
        // Extract volume
        size_t volumePos = jsonContent.find("\"volume\":");
        if (volumePos != std::string::npos) {
            size_t start = jsonContent.find_first_of("0123456789", volumePos);
            size_t end = jsonContent.find_first_not_of("0123456789", start);
            if (start != std::string::npos && end != std::string::npos) {
                quote.volume = std::stol(jsonContent.substr(start, end - start));
            }
        }
        
        // Calculate change
        quote.change = quote.currentPrice - quote.previousClose;
        if (quote.previousClose > 0) {
            quote.changePercent = (quote.change / quote.previousClose) * 100.0;
        }
        
        // Get current time
        std::time_t now = std::time(nullptr);
        std::tm* timeinfo = std::localtime(&now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        quote.lastUpdate = std::string(buffer);
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing quote JSON: " << e.what() << "\n";
    }
    
    return quote;
}

std::vector<StockData> StockDataLoader::GetRecentData(const std::string& ticker, int days) {
    // Get current date
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    
    // Calculate start date (N days ago)
    std::tm startDate = *timeinfo;
    startDate.tm_mday -= days;
    std::mktime(&startDate);
    
    // Format dates
    char endDateStr[11];
    std::strftime(endDateStr, sizeof(endDateStr), "%Y-%m-%d", timeinfo);
    
    char startDateStr[11];
    std::strftime(startDateStr, sizeof(startDateStr), "%Y-%m-%d", &startDate);
    
    return LoadFromAPI(ticker, std::string(startDateStr), std::string(endDateStr));
}

LiveQuote StockDataLoader::ParseQuoteJSON(const std::string& jsonContent, const std::string& ticker) {
    // This is a simplified parser - for production, use a JSON library like nlohmann/json
    return GetLatestQuote(ticker); // Fallback to main function
}
