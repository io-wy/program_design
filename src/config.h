#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

struct Config {
    int defaultShelfLifeDays = 730;  // 默认保质期（天）
    int nearExpiryThresholdDays = 30; // 临期阈值（天）
    std::unordered_map<std::string, int> shelfLifeByCategory; // 分类保质期
};

// 配置相关函数
Config loadConfig(const std::string &configFilePath);
int shelfLifeFor(const Config &config, const std::string &category);

#endif // CONFIG_H