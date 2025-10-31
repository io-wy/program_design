#ifndef DRUG_H
#define DRUG_H

#include <string>
#include <ctime>

struct Drug {
    std::string name;           
    std::string category;       
    std::string manufacturer;   
    std::string specification;  // 药品规格
    std::string productionDate; // 生产日期
    int stock = 0;              
    int totalSold = 0;          
    int shelfLifeDays = 0;    // 保质期（天）
    int nearExpiryThresholdDays = 0; // 临期阈值（天）
};

bool parseDate(const std::string &dateStr, std::tm &tmOut);
std::time_t toTimeT(std::tm &tmVal);
std::time_t addDays(std::time_t start, int days);
int daysUntil(std::time_t future);

#endif 