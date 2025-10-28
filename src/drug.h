#ifndef DRUG_H
#define DRUG_H

#include <string>
#include <ctime>

struct Drug {
    std::string name;           // 名称
    std::string category;       // 分类
    std::string manufacturer;   // 生产厂家
    std::string specification;  // 药品规格
    std::string productionDate; // 生产日期 YYYY-MM-DD
    int stock = 0;              // 库存量
    int totalSold = 0;          // 累计销售量
    int shelfLifeDays = 60;    // 保质期（天）
    int nearExpiryThresholdDays = 10; // 临期阈值（天）
};

// 日期相关工具函数
bool parseDate(const std::string &dateStr, std::tm &tmOut);
std::time_t toTimeT(std::tm &tmVal);
std::time_t addDays(std::time_t start, int days);
int daysUntil(std::time_t future);

#endif // DRUG_H