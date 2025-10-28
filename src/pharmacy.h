#ifndef PHARMACY_H
#define PHARMACY_H

#include "drug.h"
#include <vector>
#include <string>

class Pharmacy {
public:
    Pharmacy(const std::string &dataFile);
    void run();

private:
    std::vector<Drug> drugs;
    std::string dataFilePath;

    void loadData();
    void saveData();
    void menuLoop();
    void onExit();
    
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