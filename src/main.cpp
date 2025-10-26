#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

struct Drug {
    std::string name;           // 名称
    std::string category;       // 分类
    std::string productionDate; // 生产日期 YYYY-MM-DD
    int stock = 0;              // 库存量
    int totalSold = 0;          // 累计销售量
};

struct Config {
    int defaultShelfLifeDays = 730;  // 默认保质期（天）
    int nearExpiryThresholdDays = 30; // 临期阈值（天）
    std::unordered_map<std::string, int> shelfLifeByCategory; // 分类保质期
};

// 解析 YYYY-MM-DD 到 std::tm
bool parseDate(const std::string &dateStr, std::tm &tmOut) {
    std::istringstream iss(dateStr);
    iss >> std::get_time(&tmOut, "%Y-%m-%d");
    return !iss.fail();
}

// 将 tm 转为 time_t（本地时区）
std::time_t toTimeT(std::tm &tmVal) {
    tmVal.tm_hour = 0; tmVal.tm_min = 0; tmVal.tm_sec = 0;
    tmVal.tm_isdst = -1;
    return std::mktime(&tmVal);
}

// 给定起始日期与增加的天数，返回到期日期的 time_t
std::time_t addDays(std::time_t start, int days) {
    const long secondsPerDay = 24L * 60L * 60L;
    return start + static_cast<long>(days) * secondsPerDay;
}

// 距离未来某时间点剩余天数（负数表示已过期）
int daysUntil(std::time_t future) {
    std::time_t now = std::time(nullptr);
    double diff = std::difftime(future, now);
    return static_cast<int>(diff / (24 * 3600));
}

class Pharmacy {
public:
    Pharmacy(const std::string &dataFile, const std::string &configFile)
        : dataFilePath(dataFile), configFilePath(configFile) {}

    void run() {
        loadConfig();
        loadData();
        menuLoop();
    }

private:
    std::vector<Drug> drugs;
    Config config;
    std::string dataFilePath;
    std::string configFilePath;

    void loadConfig() {
        std::ifstream in(configFilePath);
        if (!in) {
            std::cout << "[提示] 未找到配置文件，使用默认配置。\n";
            return;
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
    }

    void loadData() {
        std::ifstream in(dataFilePath);
        if (!in) {
            std::cout << "[提示] 未找到数据文件，将在保存时创建：" << dataFilePath << "\n";
            return;
        }
        std::string header;
        std::getline(in, header); // 跳过表头
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string token;
            Drug d;
            // name
            if (!std::getline(iss, token, ',')) continue; d.name = token;
            // category
            if (!std::getline(iss, token, ',')) continue; d.category = token;
            // production_date
            if (!std::getline(iss, token, ',')) continue; d.productionDate = token;
            // stock
            if (!std::getline(iss, token, ',')) continue; d.stock = std::stoi(token);
            // total_sold
            if (!std::getline(iss, token, ',')) continue; d.totalSold = std::stoi(token);
            drugs.push_back(d);
        }
        std::cout << "[数据] 载入药品记录数：" << drugs.size() << "\n";
    }

    void saveData() {
        std::ofstream out(dataFilePath);
        if (!out) { std::cout << "[错误] 无法写入数据文件：" << dataFilePath << "\n"; return; }
        out << "name,category,production_date,stock,total_sold\n";
        for (const auto &d : drugs) {
            out << d.name << "," << d.category << "," << d.productionDate << ","
                << d.stock << "," << d.totalSold << "\n";
        }
        std::cout << "[数据] 保存成功，共 " << drugs.size() << " 条记录。\n";
    }

    int shelfLifeFor(const std::string &category) const {
        auto it = config.shelfLifeByCategory.find(category);
        if (it != config.shelfLifeByCategory.end()) return it->second;
        return config.defaultShelfLifeDays;
    }

    void menuLoop() {
        while (true) {
            std::cout << "\n===== 药房销售系统（命令行） =====\n";
            std::cout << "1. 新增药品\n";
            std::cout << "2. 查询药品（按名称）\n";
            std::cout << "3. 查询药品（按分类）\n";
            std::cout << "4. 修改药品\n";
            std::cout << "5. 删除药品\n";
            std::cout << "6. 显示临期药品\n";
            std::cout << "7. 模拟销售\n";
            std::cout << "8. 销售统计报表\n";
            std::cout << "9. 保存数据\n";
            std::cout << "10. 重新载入数据\n";
            std::cout << "0. 退出\n";
            std::cout << "请选择操作：";
            int choice; if (!(std::cin >> choice)) return; std::cin.ignore(1024, '\n');
            switch (choice) {
                case 1: addDrug(); break;
                case 2: queryByName(); break;
                case 3: queryByCategory(); break;
                case 4: modifyDrug(); break;
                case 5: deleteDrug(); break;
                case 6: showNearExpiry(); break;
                case 7: simulateSale(); break;
                case 8: salesReport(); break;
                case 9: saveData(); break;
                case 10: drugs.clear(); loadData(); break;
                case 0: onExit(); return;
                default: std::cout << "无效选择，请重试。\n"; break;
            }
        }
    }

    void onExit() {
        std::cout << "是否保存数据再退出？(y/n)：";
        char c; std::cin >> c; if (c == 'y' || c == 'Y') saveData();
        std::cout << "已退出。\n";
    }

    void addDrug() {
        Drug d;
        std::cout << "名称："; std::getline(std::cin, d.name);
        std::cout << "分类："; std::getline(std::cin, d.category);
        std::cout << "生产日期(YYYY-MM-DD)："; std::getline(std::cin, d.productionDate);
        std::cout << "库存量："; std::cin >> d.stock; std::cin.ignore(1024, '\n');
        d.totalSold = 0;
        drugs.push_back(d);
        std::cout << "[新增] 成功。当前总记录数：" << drugs.size() << "\n";
    }

    void queryByName() {
        std::string name; std::cout << "输入名称关键字："; std::getline(std::cin, name);
        int count = 0;
        for (const auto &d : drugs) {
            if (d.name.find(name) != std::string::npos) {
                printDrug(d); count++;
            }
        }
        if (count == 0) std::cout << "[查询] 未找到匹配项。\n";
    }

    void queryByCategory() {
        std::string cat; std::cout << "输入分类："; std::getline(std::cin, cat);
        int count = 0;
        for (const auto &d : drugs) {
            if (d.category == cat) { printDrug(d); count++; }
        }
        if (count == 0) std::cout << "[查询] 未找到该分类的药品。\n";
    }

    void modifyDrug() {
        std::string name; std::cout << "输入要修改的药品名称："; std::getline(std::cin, name);
        auto it = std::find_if(drugs.begin(), drugs.end(), [&](const Drug &d){ return d.name == name; });
        if (it == drugs.end()) { std::cout << "[修改] 未找到。\n"; return; }
        Drug &d = *it;
        std::cout << "新名称(留空不改)："; std::string nv; std::getline(std::cin, nv); if (!nv.empty()) d.name = nv;
        std::cout << "新分类(留空不改)："; std::string cv; std::getline(std::cin, cv); if (!cv.empty()) d.category = cv;
        std::cout << "新生产日期(留空不改)："; std::string pv; std::getline(std::cin, pv); if (!pv.empty()) d.productionDate = pv;
        std::cout << "新库存量(-1不改)："; int sv; std::cin >> sv; std::cin.ignore(1024, '\n'); if (sv >= 0) d.stock = sv;
        std::cout << "新累计销量(-1不改)："; int tv; std::cin >> tv; std::cin.ignore(1024, '\n'); if (tv >= 0) d.totalSold = tv;
        std::cout << "[修改] 完成。\n";
    }

    void deleteDrug() {
        std::string name; std::cout << "输入要删除的药品名称："; std::getline(std::cin, name);
        auto oldSize = drugs.size();
        drugs.erase(std::remove_if(drugs.begin(), drugs.end(), [&](const Drug &d){ return d.name == name; }), drugs.end());
        if (drugs.size() == oldSize) std::cout << "[删除] 未找到。\n"; else std::cout << "[删除] 已删除。\n";
    }

    void showNearExpiry() {
        int shown = 0;
        for (const auto &d : drugs) {
            std::tm tmProd{};
            if (!parseDate(d.productionDate, tmProd)) { std::cout << "[警告] 日期格式错误：" << d.productionDate << "\n"; continue; }
            std::time_t prod = toTimeT(tmProd);
            int shelf = shelfLifeFor(d.category);
            std::time_t expiry = addDays(prod, shelf);
            int remain = daysUntil(expiry);
            if (remain <= config.nearExpiryThresholdDays) {
                printDrug(d);
                std::cout << "  剩余天数：" << remain << " (≤ " << config.nearExpiryThresholdDays << ")\n";
                shown++;
            }
        }
        if (shown == 0) std::cout << "[临期] 当前无临期药品。\n";
    }

    void simulateSale() {
        std::string name; std::cout << "销售药品名称："; std::getline(std::cin, name);
        int qty; std::cout << "销售数量："; std::cin >> qty; std::cin.ignore(1024, '\n');
        if (qty <= 0) { std::cout << "[销售] 数量需为正。\n"; return; }
        auto it = std::find_if(drugs.begin(), drugs.end(), [&](const Drug &d){ return d.name == name; });
        if (it == drugs.end()) { std::cout << "[销售] 未找到。\n"; return; }
        Drug &d = *it;
        if (d.stock < qty) { std::cout << "[销售] 库存不足，当前库存：" << d.stock << "\n"; return; }
        d.stock -= qty; d.totalSold += qty;
        std::cout << "[销售] 成功。剩余库存：" << d.stock << ", 累计销量：" << d.totalSold << "\n";
    }

    void salesReport() {
        int totalSoldAll = 0, totalStockAll = 0;
        for (const auto &d : drugs) { totalSoldAll += d.totalSold; totalStockAll += d.stock; }
        std::vector<Drug> sorted = drugs;
        std::sort(sorted.begin(), sorted.end(), [](const Drug &a, const Drug &b){ return a.totalSold > b.totalSold; });
        std::cout << "\n=== 销售统计报表 ===\n";
        std::cout << "总销量：" << totalSoldAll << ", 总库存：" << totalStockAll << "\n";
        std::cout << "销量Top（前5）：\n";
        for (size_t i = 0; i < sorted.size() && i < 5; ++i) {
            std::cout << i+1 << ". " << sorted[i].name << " [" << sorted[i].category << "]"
                      << " - 销量: " << sorted[i].totalSold << ", 库存: " << sorted[i].stock << "\n";
        }
    }

    void printDrug(const Drug &d) const {
        std::cout << "名称: " << d.name << ", 分类: " << d.category
                  << ", 生产日期: " << d.productionDate
                  << ", 库存: " << d.stock << ", 累计销量: " << d.totalSold << "\n";
    }
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::string dataPath = "data/drugs.csv";
    std::string configPath = "config.txt";
    Pharmacy app(dataPath, configPath);
    app.run();
    return 0;
}