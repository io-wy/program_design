#include "database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

static bool file_exists(const std::string &path) {
    std::ifstream f(path);
    return (bool)f;
}

FileDatabase::FileDatabase(const std::string &dataDir) : dataDir(dataDir) {}

std::string FileDatabase::drugsPath() const { return dataDir + "/drugs.csv"; }
std::string FileDatabase::usersPath() const { return dataDir + "/users.csv"; }
std::string FileDatabase::salesPath() const { return dataDir + "/sales.csv"; }

void FileDatabase::ensureDirExists() const {
#ifdef _WIN32
    _mkdir(dataDir.c_str());
#else
    mkdir(dataDir.c_str(), 0755);
#endif
}

bool FileDatabase::init() {
    ensureDirExists();
    // 初始化用户文件，若不存在则创建一个默认管理员
    if (!file_exists(usersPath())) {
        std::ofstream out(usersPath());
        if (!out) { std::cout << "[错误] 无法创建用户文件：" << usersPath() << "\n"; return false; }
        out << "username,password,role\n";
        out << "admin,admin,admin\n"; // 默认账号，建议运行后尽快更改
    }
    // 初始化销售文件（若不存在则写入表头）
    if (!file_exists(salesPath())) {
        std::ofstream out(salesPath());
        if (!out) { std::cout << "[错误] 无法创建销售记录文件：" << salesPath() << "\n"; return false; }
        out << "drug_name,quantity,timestamp,operator\n";
    }
    // 药品文件如不存在，后续保存时会创建
    return true;
}

std::vector<Drug> FileDatabase::loadDrugs() {
    std::vector<Drug> list;
    std::ifstream in(drugsPath());
    if (!in) {
        std::cout << "[提示] 未找到药品数据文件，将在保存时创建。\n";
        return list;
    }
    std::string header; std::getline(in, header);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string token; std::vector<std::string> tokens;
        while (std::getline(iss, token, ',')) tokens.push_back(token);
        if (tokens.size() < 7) continue;
        Drug d;
        d.name = tokens[0];
        d.category = tokens[1];
        d.manufacturer = tokens[2];
        d.specification = tokens[3];
        d.productionDate = tokens[4];
        d.stock = std::stoi(tokens[5]);
        d.totalSold = std::stoi(tokens[6]);
        if (tokens.size() >= 9) {
            d.shelfLifeDays = std::stoi(tokens[7]);
            d.nearExpiryThresholdDays = std::stoi(tokens[8]);
        }
        list.push_back(d);
    }
    return list;
}

bool FileDatabase::saveDrugs(const std::vector<Drug>& drugs) {
    ensureDirExists();
    std::ofstream out(drugsPath());
    if (!out) { std::cout << "[错误] 无法写入药品数据文件：" << drugsPath() << "\n"; return false; }
    out << "name,category,manufacturer,specification,production_date,stock,total_sold,shelf_life_days,near_expiry_days\n";
    for (const auto &d : drugs) {
        out << d.name << "," << d.category << "," << d.manufacturer << ","
            << d.specification << "," << d.productionDate << ","
            << d.stock << "," << d.totalSold << ","
            << d.shelfLifeDays << "," << d.nearExpiryThresholdDays << "\n";
    }
    return true;
}

std::vector<User> FileDatabase::loadUsers() {
    std::vector<User> list;
    std::ifstream in(usersPath());
    if (!in) return list;
    std::string header; std::getline(in, header);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string u, p, r;
        if (!std::getline(iss, u, ',')) continue;
        if (!std::getline(iss, p, ',')) continue;
        if (!std::getline(iss, r, ',')) continue;
        list.push_back(User{u, p, r});
    }
    return list;
}

bool FileDatabase::saveUsers(const std::vector<User>& users) {
    ensureDirExists();
    std::ofstream out(usersPath());
    if (!out) { std::cout << "[错误] 无法写入用户文件：" << usersPath() << "\n"; return false; }
    out << "username,password,role\n";
    for (const auto &u : users) {
        out << u.username << "," << u.password << "," << u.role << "\n";
    }
    return true;
}

bool FileDatabase::appendSale(const SaleRecord& record) {
    ensureDirExists();
    std::ofstream out(salesPath(), std::ios::app);
    if (!out) { std::cout << "[错误] 无法写入销售记录文件：" << salesPath() << "\n"; return false; }
    out << record.drugName << "," << record.quantity << "," << record.timestamp
        << "," << record.operatorName << "\n";
    return true;
}

std::vector<SaleRecord> FileDatabase::loadSales() {
    std::vector<SaleRecord> list;
    std::ifstream in(salesPath());
    if (!in) return list;
    std::string header; std::getline(in, header);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        SaleRecord rec; std::string qty;
        if (!std::getline(iss, rec.drugName, ',')) continue;
        if (!std::getline(iss, qty, ',')) continue; rec.quantity = std::stoi(qty);
        if (!std::getline(iss, rec.timestamp, ',')) continue;
        if (!std::getline(iss, rec.operatorName, ',')) continue;
        list.push_back(rec);
    }
    return list;
}