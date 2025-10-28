#ifndef PHARMACY_H
#define PHARMACY_H

#include "drug.h"
#include "database.h"
#ifdef HAS_SQLITE
#include "sqlite_db.h"
#endif
#include <vector>
#include <string>
#include <memory>

class Pharmacy {
public:
    Pharmacy(const std::string &dataFile);
    void run();

private:
    std::vector<Drug> drugs;
    std::string dataFilePath;
    std::string dataDir;
    std::unique_ptr<IDatabase> db;
    bool loggedIn = false;
    User currentUser;

    void loadData();
    void saveData();
    void menuLoop();
    void onExit();
    bool login();
    void viewSales();
    
    // 药品管理功能
    void addDrug();
    void queryByName();
    void queryByCategory();
    void modifyDrug();
    void deleteDrug();
    
    // 其他功能
    void showNearExpiry();
    void simulateSale();
    void salesReport();
    void printDrug(const Drug &d) const;
};

#endif // PHARMACY_H