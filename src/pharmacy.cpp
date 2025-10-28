#include "pharmacy.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
// #include <algorithm> // 由于MSVC头文件冲突，改用自实现Top5逻辑避免依赖

Pharmacy::Pharmacy(const std::string &dataFile)
    : dataFilePath(dataFile) {}

void Pharmacy::run() {
    loadData();
    menuLoop();
}

void Pharmacy::loadData() {
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
        std::vector<std::string> tokens;
        while (std::getline(iss, token, ',')) tokens.push_back(token);
        if (tokens.size() < 7) continue; // 至少需要前7列
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
        drugs.push_back(d);
    }
    std::cout << "[数据] 载入药品记录数：" << drugs.size() << "\n";
}

void Pharmacy::saveData() {
    std::ofstream out(dataFilePath);
    if (!out) { std::cout << "[错误] 无法写入数据文件：" << dataFilePath << "\n"; return; }
    out << "name,category,manufacturer,specification,production_date,stock,total_sold,shelf_life_days,near_expiry_days\n";
    for (const auto &d : drugs) {
        out << d.name << "," << d.category << "," << d.manufacturer << "," 
            << d.specification << "," << d.productionDate << ","
            << d.stock << "," << d.totalSold << ","
            << d.shelfLifeDays << "," << d.nearExpiryThresholdDays << "\n";
    }
    std::cout << "[数据] 保存成功，共 " << drugs.size() << " 条记录。\n";
}

void Pharmacy::menuLoop() {
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

void Pharmacy::onExit() {
    std::cout << "是否保存数据再退出？(y/n)：";
    char c; std::cin >> c; if (c == 'y' || c == 'Y') saveData();
    std::cout << "已退出。\n";
}

void Pharmacy::addDrug() {
    Drug d;
    std::cout << "名称："; std::getline(std::cin, d.name);
    std::cout << "分类："; std::getline(std::cin, d.category);
    std::cout << "生产厂家："; std::getline(std::cin, d.manufacturer);
    std::cout << "药品规格："; std::getline(std::cin, d.specification);
    std::cout << "生产日期(YYYY-MM-DD)："; std::getline(std::cin, d.productionDate);
    std::cout << "库存量："; std::cin >> d.stock; std::cin.ignore(1024, '\n');
    std::cout << "保质期天数："; int sh; if (std::cin >> sh) { if (sh > 0) d.shelfLifeDays = sh; } else { std::cin.clear(); } std::cin.ignore(1024, '\n');
    std::cout << "临期阈值天数："; int th; if (std::cin >> th) { if (th > 0) d.nearExpiryThresholdDays = th; } else { std::cin.clear(); } std::cin.ignore(1024, '\n');
    d.totalSold = 0;
    drugs.push_back(d);
    std::cout << "[新增] 成功。当前总记录数：" << drugs.size() << "\n";
}

void Pharmacy::queryByName() {
    std::string name; std::cout << "输入名称关键字："; std::getline(std::cin, name);
    int count = 0;
    for (const auto &d : drugs) {
        if (d.name.find(name) != std::string::npos) {
            printDrug(d); count++;
        }
    }
    if (count == 0) std::cout << "[查询] 未找到匹配项。\n";
}

void Pharmacy::queryByCategory() {
    std::string cat; std::cout << "输入分类："; std::getline(std::cin, cat);
    int count = 0;
    for (const auto &d : drugs) {
        if (d.category == cat) { printDrug(d); count++; }
    }
    if (count == 0) std::cout << "[查询] 未找到该分类的药品。\n";
}

void Pharmacy::modifyDrug() {
    std::string name; std::cout << "输入要修改的药品名称："; std::getline(std::cin, name);
    auto it = drugs.begin();
    while (it != drugs.end() && it->name != name) ++it;
    if (it == drugs.end()) { std::cout << "[修改] 未找到。\n"; return; }
    Drug &d = *it;
    std::cout << "新名称(留空不改)："; std::string nv; std::getline(std::cin, nv); if (!nv.empty()) d.name = nv;
    std::cout << "新分类(留空不改)："; std::string cv; std::getline(std::cin, cv); if (!cv.empty()) d.category = cv;
    std::cout << "新生产厂家(留空不改)："; std::string mv; std::getline(std::cin, mv); if (!mv.empty()) d.manufacturer = mv;
    std::cout << "新药品规格(留空不改)："; std::string sv; std::getline(std::cin, sv); if (!sv.empty()) d.specification = sv;
    std::cout << "新生产日期(留空不改)："; std::string pv; std::getline(std::cin, pv); if (!pv.empty()) d.productionDate = pv;
    std::cout << "新库存量(-1不改)："; int stv; std::cin >> stv; std::cin.ignore(1024, '\n'); if (stv >= 0) d.stock = stv;
    std::cout << "新累计销量(-1不改)："; int tv; std::cin >> tv; std::cin.ignore(1024, '\n'); if (tv >= 0) d.totalSold = tv;
    std::cout << "[修改] 完成。\n";
}

void Pharmacy::deleteDrug() {
    std::string name; std::cout << "输入要删除的药品名称："; std::getline(std::cin, name);
    auto oldSize = drugs.size();
    for (auto it = drugs.begin(); it != drugs.end(); ) {
        if (it->name == name) it = drugs.erase(it);
        else ++it;
    }
    if (drugs.size() == oldSize) std::cout << "[删除] 未找到。\n"; else std::cout << "[删除] 已删除。\n";
}

void Pharmacy::showNearExpiry() {
    int shown = 0;
    for (const auto &d : drugs) {
        std::tm tmProd{};
        if (!parseDate(d.productionDate, tmProd)) { std::cout << "[警告] 日期格式错误：" << d.productionDate << "\n"; continue; }
        std::time_t prod = toTimeT(tmProd);
        int shelf = d.shelfLifeDays;
        std::time_t expiry = addDays(prod, shelf);
        int remain = daysUntil(expiry);
        if (remain <= d.nearExpiryThresholdDays) {
            printDrug(d);
            std::cout << "  剩余天数：" << remain << " (≤ " << d.nearExpiryThresholdDays << ")\n";
            shown++;
        }
    }
    if (shown == 0) std::cout << "[临期] 当前无临期药品。\n";
}

void Pharmacy::simulateSale() {
    std::string name; std::cout << "销售药品名称："; std::getline(std::cin, name);
    int qty; std::cout << "销售数量："; std::cin >> qty; std::cin.ignore(1024, '\n');
    if (qty <= 0) { std::cout << "[销售] 数量需为正。\n"; return; }
    auto it = drugs.begin();
    while (it != drugs.end() && it->name != name) ++it;
    if (it == drugs.end()) { std::cout << "[销售] 未找到。\n"; return; }
    Drug &d = *it;
    if (d.stock < qty) { std::cout << "[销售] 库存不足，当前库存：" << d.stock << "\n"; return; }
    d.stock -= qty; d.totalSold += qty;
    std::cout << "[销售] 成功。剩余库存：" << d.stock << ", 累计销量：" << d.totalSold << "\n";
}

void Pharmacy::salesReport() {
    int totalSoldAll = 0, totalStockAll = 0;
    for (const auto &d : drugs) { totalSoldAll += d.totalSold; totalStockAll += d.stock; }
    // 计算销量Top5（不依赖<algorithm>）
    std::vector<Drug> topList;
    topList.reserve(5);
    for (const auto &d : drugs) {
        size_t pos = 0;
        while (pos < topList.size() && topList[pos].totalSold >= d.totalSold) pos++;
        if (pos <= topList.size()) topList.insert(topList.begin() + pos, d);
        if (topList.size() > 5) topList.pop_back();
    }
    std::cout << "\n=== 销售统计报表 ===\n";
    std::cout << "总销量：" << totalSoldAll << ", 总库存：" << totalStockAll << "\n";
    std::cout << "销量Top（前5）：\n";
    for (size_t i = 0; i < topList.size(); ++i) {
        std::cout << i+1 << ". " << topList[i].name << " [" << topList[i].category << "]"
                  << " - 销量: " << topList[i].totalSold << ", 库存: " << topList[i].stock << "\n";
    }
}

void Pharmacy::printDrug(const Drug &d) const {
    std::cout << "名称: " << d.name << ", 分类: " << d.category
              << ", 生产厂家: " << d.manufacturer << ", 规格: " << d.specification
              << ", 生产日期: " << d.productionDate
              << ", 库存: " << d.stock << ", 累计销量: " << d.totalSold << "\n";
}