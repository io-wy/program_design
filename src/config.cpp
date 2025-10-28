#include "config.h"
#include <iostream>
#include <fstream>
#include <string>

Config loadConfig(const std::string &configFilePath) {
    Config config;
    std::ifstream in(configFilePath);
    if (!in) {
        std::cout << "[提示] 未找到配置文件，使用默认配置。\n";
        return config;
    }
    
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        
        if (key == "default_shelf_life_days") {
            config.defaultShelfLifeDays = std::stoi(val);
        } else if (key == "near_expiry_threshold_days") {
            config.nearExpiryThresholdDays = std::stoi(val);
        } else if (key.rfind("shelf_life.", 0) == 0) {
            std::string cat = key.substr(std::string("shelf_life.").size());
            config.shelfLifeByCategory[cat] = std::stoi(val);
        }
    }
    
    std::cout << "[配置] 默认保质期: " << config.defaultShelfLifeDays
              << " 天，临期阈值: " << config.nearExpiryThresholdDays << " 天\n";
    
    return config;
}

int shelfLifeFor(const Config &config, const std::string &category) {
    auto it = config.shelfLifeByCategory.find(category);
    if (it != config.shelfLifeByCategory.end()) return it->second;
    return config.defaultShelfLifeDays;
}