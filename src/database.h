#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "drug.h"

struct User {
    std::string username;
    std::string password; // 简化处理，明文存储；生产建议使用哈希
    std::string role;     // "admin" 或 "clerk"
};

struct SaleRecord {
    std::string drugName;
    int quantity = 0;
    std::string timestamp;    
    std::string operatorName;  // 执行销售的用户
    std::string type = "SALE"; // 交易类型：SALE/RETURN/WASTAGE
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual bool init() = 0;

    virtual std::vector<Drug> loadDrugs() = 0;
    virtual bool saveDrugs(const std::vector<Drug>& drugs) = 0;

    virtual std::vector<User> loadUsers() = 0;
    virtual bool saveUsers(const std::vector<User>& users) = 0;

    virtual bool appendSale(const SaleRecord& record) = 0;
    virtual std::vector<SaleRecord> loadSales() = 0;
};

#endif // DATABASE_H