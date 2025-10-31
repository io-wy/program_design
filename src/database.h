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

class FileDatabase : public IDatabase {
public:
    explicit FileDatabase(const std::string &dataDir);
    bool init() override;

    std::vector<Drug> loadDrugs() override;
    bool saveDrugs(const std::vector<Drug>& drugs) override;

    std::vector<User> loadUsers() override;
    bool saveUsers(const std::vector<User>& users) override;

    bool appendSale(const SaleRecord& record) override;
    std::vector<SaleRecord> loadSales() override;

private:
    std::string dataDir;
    std::string drugsPath() const;
    std::string usersPath() const;
    std::string salesPath() const;
    void ensureDirExists() const;
};

#endif // DATABASE_H