#include "Config.h"
#include <fstream>
#include <iostream>
#include <sstream>

Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

bool Config::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << filepath << "\n";
        std::cerr << "Using default configuration.\n";
        return false;
    }
    
    // Simple JSON parser (for basic config)
    // For production, use a proper JSON library like nlohmann/json
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == '/') continue;
        
        // Simple key-value extraction (basic implementation)
        if (line.find("\"default_ticker\"") != std::string::npos) {
            size_t start = line.find(':');
            if (start != std::string::npos) {
                size_t quote1 = line.find('"', start);
                size_t quote2 = line.find('"', quote1 + 1);
                if (quote1 != std::string::npos && quote2 != std::string::npos) {
                    defaultTicker = line.substr(quote1 + 1, quote2 - quote1 - 1);
                }
            }
        }
        // Add more parsing as needed
        // For now, this is a basic implementation
    }
    
    file.close();
    std::cout << "Configuration loaded from " << filepath << "\n";
    return true;
}

