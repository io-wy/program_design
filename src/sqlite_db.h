#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include "database.h"
#include <string>

class SqliteDatabase : public IDatabase {
public:
    explicit SqliteDatabase(const std::string &dbPath);
    ~SqliteDatabase();

    bool init() override;

    std::vector<Drug> loadDrugs() override;
    bool saveDrugs(const std::vector<Drug>& drugs) override;

    std::vector<User> loadUsers() override;
    bool saveUsers(const std::vector<User>& users) override;

    bool appendSale(const SaleRecord& record) override;
    std::vector<SaleRecord> loadSales() override;

private:
    std::string path;
    void *dbHandle = nullptr; // sqlite3*，使用void*避免在头文件包含sqlite3.h
    bool exec(const std::string &sql);
};

#endif // SQLITE_DB_H
