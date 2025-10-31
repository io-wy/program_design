#include "pharmacy.h"
#include <iostream>
#include <fstream> // 仅用于极少数文件操作（如未来扩展）；当前不依赖CSV
#include <iomanip>
#include <unordered_map>
#include <map>
// #include <algorithm> // 由于MSVC头文件冲突，改用自实现Top5逻辑避免依赖
#include <ctime>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

static std::string __dir_from_path(const std::string &path) {
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return std::string(".");
    return path.substr(0, pos);
}

Pharmacy::Pharmacy(const std::string &dataFile)
    : dataFilePath(dataFile) {
    dataDir = __dir_from_path(dataFilePath);
    if (dataDir.empty() || dataDir == ".") dataDir = "data";
    std::string dbPath = dataDir + "/pharmacy.db";
    db = std::make_unique<SqliteDatabase>(dbPath);
}

void Pharmacy::run() {
    if (!db->init()) { std::cout << "[错误] SQLite 初始化失败。\n"; return; }
    if (!login()) { std::cout << "[登录] 失败，程序退出。\n"; return; }
    loadData();
    menuLoop();
}

void Pharmacy::loadData() {
    drugs = db->loadDrugs();
    std::cout << "[数据] 载入药品记录数：" << drugs.size() << "\n";
}

void Pharmacy::saveData() {
    if (db->saveDrugs(drugs)) {
        std::cout << "[数据] 保存成功，共 " << drugs.size() << " 条记录。\n";
    } else {
        std::cout << "[错误] 保存失败。\n";
    }
}

void Pharmacy::menuLoop() {
    while (true) {
        std::cout << "\n===== 药房销售系统（命令行） =====\n";
        std::cout << "登录用户：" << currentUser.username << " (" << currentUser.role << ")\n";
        std::cout << "1. 新增药品\n";
        std::cout << "2. 查询药品（按名称）\n";
        std::cout << "3. 查询药品（按分类）\n";
        std::cout << "4. 展示所有药品\n";
        std::cout << "5. 修改药品\n";
        std::cout << "6. 删除药品\n";
        std::cout << "7. 显示临期药品\n";
        std::cout << "8. 显示过期药品数量\n";
        std::cout << "9. 模拟销售\n";
        std::cout << "10. 销售统计报表\n";
        std::cout << "11. 保存数据\n";
        std::cout << "12. 重新载入数据\n";
        std::cout << "13. 查看销售记录\n";
        std::cout << "14. 退货处理\n";
        std::cout << "15. 报损处理\n";
        std::cout << "16. 畅销/滞销分析\n";
        std::cout << "17. 品类销售趋势\n";
        std::cout << "0. 退出\n";
        std::cout << "请选择操作：";
        int choice; if (!(std::cin >> choice)) return; std::cin.ignore(1024, '\n');
        switch (choice) {
            case 1: addDrug(); break;
            case 2: queryByName(); break;
            case 3: queryByCategory(); break;
            case 4: showAllDrugs(); break;
            case 5: if (currentUser.role == "admin") modifyDrug(); else std::cout << "[权限] 仅管理员可修改。\n"; break;
            case 6: if (currentUser.role == "admin") deleteDrug(); else std::cout << "[权限] 仅管理员可删除。\n"; break;
            case 7: showNearExpiry(); break;
            case 8: showExpiredCount(); break;
            case 9: simulateSale(); break;
            case 10: salesReport(); break;
            case 11: saveData(); break;
            case 12: drugs.clear(); loadData(); break;
            case 13: viewSales(); break;
            case 14: processReturn(); break;
            case 15: processWastage(); break;
            case 16: analyzeTopBottom(); break;
            case 17: categorySalesTrend(); break;
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

void Pharmacy::showAllDrugs() {
    if (drugs.empty()) {
        std::cout << "[展示] 暂无药品记录。\n";
        return;
    }
    
    std::cout << "\n=== 所有药品列表 ===\n";
    std::cout << "共 " << drugs.size() << " 种药品：\n\n";
    
    int index = 1;
    for (const auto &d : drugs) {
        std::cout << "[" << index++ << "] ";
        printDrug(d);
    }
    std::cout << "\n=== 展示完毕 ===\n";
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
            if (remain < 0) {
                std::cout << "  状态：已过期 " << (-remain) << " 天 (≤ " << d.nearExpiryThresholdDays << ")\n";
            } else {
                std::cout << "  剩余天数：" << remain << " (≤ " << d.nearExpiryThresholdDays << ")\n";
            }
            shown++;
        }
    }
    if (shown == 0) std::cout << "[临期] 当前无临期药品。\n";
}

void Pharmacy::showExpiredCount() {
    int expiredCount = 0;
    for (const auto &d : drugs) {
        std::tm tmProd{};
        if (!parseDate(d.productionDate, tmProd)) { 
            std::cout << "[警告] 日期格式错误：" << d.productionDate << "\n"; 
            continue; 
        }
        std::time_t prod = toTimeT(tmProd);
        int shelf = d.shelfLifeDays;
        std::time_t expiry = addDays(prod, shelf);
        int remain = daysUntil(expiry);
        if (remain < 0) {
            expiredCount++;
        }
    }
    std::cout << "\n=== 过期药品统计 ===\n";
    std::cout << "过期药品总数：" << expiredCount << " 种\n";
    if (expiredCount > 0) {
        std::cout << "建议及时处理过期药品！\n";
    } else {
        std::cout << "当前无过期药品。\n";
    }
    std::cout << "===================\n\n";
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
    // 记录销售到数据库
    std::time_t now = std::time(nullptr);
    std::tm tmNow{};
#ifdef _WIN32
    localtime_s(&tmNow, &now);
#else
    tmNow = *std::localtime(&now);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmNow);
    SaleRecord rec{ d.name, qty, std::string(buf), currentUser.username };
    rec.type = "SALE";
    db->appendSale(rec);
}

void Pharmacy::salesReport() {
    if (drugs.empty()) {
        std::cout << "\n=== 销售统计报表 ===\n";
        std::cout << "暂无药品数据。\n";
        std::cout << "==================\n\n";
        return;
    }
    
    int totalSoldAll = 0, totalStockAll = 0;
    for (const auto &d : drugs) { 
        totalSoldAll += d.totalSold; 
        totalStockAll += d.stock; 
    }
    
    // bubble sort
    std::vector<Drug> allDrugs = drugs;
    
    for (size_t i = 0; i < allDrugs.size(); ++i) {
        for (size_t j = 0; j < allDrugs.size() - 1 - i; ++j) {
            if (allDrugs[j].totalSold < allDrugs[j + 1].totalSold) {
                Drug temp = allDrugs[j];
                allDrugs[j] = allDrugs[j + 1];
                allDrugs[j + 1] = temp;
            }
        }
    }
    
    std::cout << "\n=== 销售统计报表 ===\n";
    std::cout << "药品总数：" << drugs.size() << " 种\n";
    std::cout << "总销量：" << totalSoldAll << ", 总库存：" << totalStockAll << "\n\n";
    std::cout << "所有药品销售排行（按销量从高到低）：\n";
    std::cout << "序号 | 药品名称 | 分类 | 销量 | 库存\n";
    std::cout << "----------------------------------------\n";
    
    for (size_t i = 0; i < allDrugs.size(); ++i) {
        std::cout << std::setw(4) << (i + 1) << " | " 
                  << std::setw(12) << allDrugs[i].name << " | "
                  << std::setw(8) << allDrugs[i].category << " | "
                  << std::setw(4) << allDrugs[i].totalSold << " | "
                  << std::setw(4) << allDrugs[i].stock << "\n";
    }
    std::cout << "==================\n\n";
}

void Pharmacy::printDrug(const Drug &d) const {
    std::cout << "名称: " << d.name << ", 分类: " << d.category
              << ", 生产厂家: " << d.manufacturer << ", 规格: " << d.specification
              << ", 生产日期: " << d.productionDate
              << ", 库存: " << d.stock << ", 累计销量: " << d.totalSold << "\n";
}

std::string Pharmacy::getHiddenPassword() {
    std::string password;
    char ch;
    
#ifdef _WIN32
    // Windows系统使用_getch()
    while ((ch = _getch()) != '\r') {  // '\r' 是回车键
        if (ch == '\b') {  // 退格键
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";  // 删除屏幕上的字符
            }
        } else if (ch >= 32 && ch <= 126) {  // 可打印字符
            password += ch;
            std::cout << '*';  // 显示星号
        }
    }
#else
    // Linux/Unix系统
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    while ((ch = getchar()) != '\n') {
        if (ch == 127 || ch == '\b') {  // 退格键
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else if (ch >= 32 && ch <= 126) {
            password += ch;
            std::cout << '*';
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    
    std::cout << std::endl;  // 换行
    return password;
}

bool Pharmacy::login() {
    auto users = db->loadUsers();
    if (users.empty()) { std::cout << "[登录] 无用户记录。\n"; return false; }
    for (int attempt = 0; attempt < 3; ++attempt) {
        std::string u, p; 
        std::cout << "用户名："; 
        std::getline(std::cin, u); 
        std::cout << "密码："; 
        p = getHiddenPassword();
        for (const auto &user : users) {
            if (user.username == u && user.password == p) {
                currentUser = user; loggedIn = true; std::cout << "[登录] 成功。\n"; return true;
            }
        }
        std::cout << "[登录] 账号或密码错误。\n";
    }
    return false;
}

void Pharmacy::viewSales() {
    auto list = db->loadSales();
    if (list.empty()) { std::cout << "[销售] 暂无记录。\n"; return; }
    std::cout << "\n=== 销售记录（最新在后） ===\n";
    for (const auto &rec : list) {
        std::cout << rec.timestamp << " | 药品：" << rec.drugName << " | 数量：" << rec.quantity
                  << " | 操作员：" << rec.operatorName << " | 类型：" << rec.type << "\n";
    }
}

// 退货处理：库存回滚、销量扣减，记录到sales（type=RETURN，数量为负值）
void Pharmacy::processReturn() {
    std::string name; std::cout << "退货药品名称："; std::getline(std::cin, name);
    int qty; std::cout << "退货数量："; std::cin >> qty; std::cin.ignore(1024, '\n');
    if (qty <= 0) { std::cout << "[退货] 数量需为正。\n"; return; }
    auto it = drugs.begin();
    while (it != drugs.end() && it->name != name) ++it;
    if (it == drugs.end()) { std::cout << "[退货] 未找到。\n"; return; }
    Drug &d = *it;
    // 退货回滚销量、增加库存
    d.stock += qty;
    if (d.totalSold < qty) d.totalSold = 0; else d.totalSold -= qty;
    std::cout << "[退货] 成功。库存：" << d.stock << ", 累计销量：" << d.totalSold << "\n";

    // 记录到sales（负数量）
    std::time_t now = std::time(nullptr);
    std::tm tmNow{};
#ifdef _WIN32
    localtime_s(&tmNow, &now);
#else
    tmNow = *std::localtime(&now);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmNow);
    SaleRecord rec{ d.name, -qty, std::string(buf), currentUser.username };
    rec.type = "RETURN";
    db->appendSale(rec);
}

// 报损处理：扣减库存，不影响累计销量，记录到sales（type=WASTAGE，负数量）
void Pharmacy::processWastage() {
    std::string name; std::cout << "报损药品名称："; std::getline(std::cin, name);
    int qty; std::cout << "报损数量："; std::cin >> qty; std::cin.ignore(1024, '\n');
    if (qty <= 0) { std::cout << "[报损] 数量需为正。\n"; return; }
    auto it = drugs.begin();
    while (it != drugs.end() && it->name != name) ++it;
    if (it == drugs.end()) { std::cout << "[报损] 未找到。\n"; return; }
    Drug &d = *it;
    if (d.stock < qty) { std::cout << "[报损] 库存不足，当前库存：" << d.stock << "\n"; return; }
    d.stock -= qty;
    std::cout << "[报损] 成功。剩余库存：" << d.stock << ", 累计销量：" << d.totalSold << "\n";

    // 记录到sales（负数量）
    std::time_t now = std::time(nullptr);
    std::tm tmNow{};
#ifdef _WIN32
    localtime_s(&tmNow, &now);
#else
    tmNow = *std::localtime(&now);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmNow);
    SaleRecord rec{ d.name, -qty, std::string(buf), currentUser.username };
    rec.type = "WASTAGE";
    db->appendSale(rec);
}

// 畅销/滞销分析：输出前10畅销与后10滞销（按累计销量）
void Pharmacy::analyzeTopBottom() {
    if (drugs.empty()) { std::cout << "[分析] 暂无药品数据。\n"; return; }
    std::vector<Drug> list = drugs;
    // 冒泡排序按销量从高到低
    for (size_t i = 0; i < list.size(); ++i) {
        for (size_t j = 0; j < list.size() - 1 - i; ++j) {
            if (list[j].totalSold < list[j + 1].totalSold) {
                Drug tmp = list[j]; list[j] = list[j + 1]; list[j + 1] = tmp;
            }
        }
    }
    size_t topN = list.size() < 10 ? list.size() : 10;
    std::cout << "\n=== 畅销 TOP" << topN << " ===\n";
    for (size_t i = 0; i < topN; ++i) {
        std::cout << std::setw(2) << (i+1) << ". " << list[i].name << " | 分类:" << list[i].category
                  << " | 销量:" << list[i].totalSold << " | 库存:" << list[i].stock << "\n";
    }
    // 滞销：从末尾取后10（销量低）
    size_t bottomN = topN;
    std::cout << "\n=== 滞销 TOP" << bottomN << " ===\n";
    for (size_t k = 0; k < bottomN; ++k) {
        const auto &d = list[list.size() - 1 - k];
        std::cout << std::setw(2) << (k+1) << ". " << d.name << " | 分类:" << d.category
                  << " | 销量:" << d.totalSold << " | 库存:" << d.stock << "\n";
    }
}

// 品类销售趋势：按月汇总每个分类的净销售量（SALE-RETURN），忽略报损
void Pharmacy::categorySalesTrend() {
    auto sales = db->loadSales();
    if (sales.empty()) { std::cout << "[趋势] 暂无销售记录。\n"; return; }
    // 名称->分类映射
    std::unordered_map<std::string, std::string> nameToCat;
    for (const auto &d : drugs) nameToCat[d.name] = d.category;

    // 分类 -> (YYYY-MM -> 数量)
    std::map<std::string, std::map<std::string, int>> catMonth;
    for (const auto &r : sales) {
        if (r.timestamp.size() < 7) continue;
        std::string ym = r.timestamp.substr(0, 7); // YYYY-MM
        std::string cat = nameToCat.count(r.drugName) ? nameToCat[r.drugName] : std::string("未知");
        if (r.type == "WASTAGE") continue; // 报损不计入销售趋势
        catMonth[cat][ym] += r.quantity; // RETURN 为负数
    }

    std::cout << "\n=== 品类销售趋势（按月） ===\n";
    for (const auto &catEntry : catMonth) {
        std::cout << "[" << catEntry.first << "]\n";
        for (const auto &mEntry : catEntry.second) {
            std::cout << "  " << mEntry.first << " : " << mEntry.second << "\n";
        }
    }
    std::cout << "==========================\n";
}