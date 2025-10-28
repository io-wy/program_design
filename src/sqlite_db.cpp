#include "sqlite_db.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

static std::string dir_from_path_sql(const std::string &path) {
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return std::string(".");
    return path.substr(0, pos);
}

static void ensure_dir_exists(const std::string &dir) {
#ifdef _WIN32
    _mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
}

SqliteDatabase::SqliteDatabase(const std::string &dbPath) : path(dbPath) {}

SqliteDatabase::~SqliteDatabase() {
    if (dbHandle) {
        sqlite3_close(static_cast<sqlite3*>(dbHandle));
        dbHandle = nullptr;
    }
}

bool SqliteDatabase::exec(const std::string &sql) {
    char *errmsg = nullptr;
    int rc = sqlite3_exec(static_cast<sqlite3*>(dbHandle), sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::cout << "[SQLite] 执行失败: " << (errmsg ? errmsg : "") << "\n";
        if (errmsg) sqlite3_free(errmsg);
        return false;
    }
    return true;
}

bool SqliteDatabase::init() {
    std::string dir = dir_from_path_sql(path);
    if (!dir.empty() && dir != ".") ensure_dir_exists(dir);

    sqlite3 *db = nullptr;
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cout << "[SQLite] 打开数据库失败: " << sqlite3_errmsg(db) << "\n";
        if (db) sqlite3_close(db);
        return false;
    }
    dbHandle = db;

    // 建表
    exec("CREATE TABLE IF NOT EXISTS drugs (\n"
         "name TEXT PRIMARY KEY,\n"
         "category TEXT, manufacturer TEXT, specification TEXT, production_date TEXT,\n"
         "stock INTEGER, total_sold INTEGER, shelf_life_days INTEGER, near_expiry_days INTEGER\n"
         ");");

    exec("CREATE TABLE IF NOT EXISTS users (\n"
         "username TEXT PRIMARY KEY, password TEXT, role TEXT\n"
         ");");

    exec("CREATE TABLE IF NOT EXISTS sales (\n"
         "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
         "drug_name TEXT, quantity INTEGER, timestamp TEXT, operator TEXT\n"
         ");");

    // 默认管理员
    const char *sqlCount = "SELECT COUNT(*) FROM users";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sqlCount, -1, &stmt, nullptr) == SQLITE_OK) {
        int rc2 = sqlite3_step(stmt);
        int count = (rc2 == SQLITE_ROW) ? sqlite3_column_int(stmt, 0) : 0;
        sqlite3_finalize(stmt);
        if (count == 0) {
            exec("INSERT INTO users(username, password, role) VALUES('admin','admin','admin')");
        }
    }
    return true;
}

std::vector<Drug> SqliteDatabase::loadDrugs() {
    std::vector<Drug> list;
    const char *sql = "SELECT name, category, manufacturer, specification, production_date, stock, total_sold, shelf_life_days, near_expiry_days FROM drugs";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Drug d;
        d.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        d.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        d.manufacturer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        d.specification = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        d.productionDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        d.stock = sqlite3_column_int(stmt, 5);
        d.totalSold = sqlite3_column_int(stmt, 6);
        d.shelfLifeDays = sqlite3_column_int(stmt, 7);
        d.nearExpiryThresholdDays = sqlite3_column_int(stmt, 8);
        list.push_back(d);
    }
    sqlite3_finalize(stmt);
    return list;
}

bool SqliteDatabase::saveDrugs(const std::vector<Drug>& drugs) {
    exec("BEGIN TRANSACTION");
    exec("DELETE FROM drugs");
    const char *sql = "INSERT INTO drugs(name, category, manufacturer, specification, production_date, stock, total_sold, shelf_life_days, near_expiry_days) VALUES(?,?,?,?,?,?,?,?,?)";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    for (const auto &d : drugs) {
        sqlite3_bind_text(stmt, 1, d.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, d.category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, d.manufacturer.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, d.specification.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, d.productionDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, d.stock);
        sqlite3_bind_int(stmt, 7, d.totalSold);
        sqlite3_bind_int(stmt, 8, d.shelfLifeDays);
        sqlite3_bind_int(stmt, 9, d.nearExpiryThresholdDays);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) { sqlite3_finalize(stmt); exec("ROLLBACK"); return false; }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);
    exec("COMMIT");
    return true;
}

std::vector<User> SqliteDatabase::loadUsers() {
    std::vector<User> list;
    const char *sql = "SELECT username, password, role FROM users";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        list.push_back(u);
    }
    sqlite3_finalize(stmt);
    return list;
}

bool SqliteDatabase::saveUsers(const std::vector<User>& users) {
    exec("BEGIN TRANSACTION");
    exec("DELETE FROM users");
    const char *sql = "INSERT INTO users(username, password, role) VALUES(?,?,?)";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    for (const auto &u : users) {
        sqlite3_bind_text(stmt, 1, u.username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, u.password.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, u.role.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) { sqlite3_finalize(stmt); exec("ROLLBACK"); return false; }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);
    exec("COMMIT");
    return true;
}

bool SqliteDatabase::appendSale(const SaleRecord& record) {
    const char *sql = "INSERT INTO sales(drug_name, quantity, timestamp, operator) VALUES(?,?,?,?)";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, record.drugName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, record.quantity);
    sqlite3_bind_text(stmt, 3, record.timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, record.operatorName.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::vector<SaleRecord> SqliteDatabase::loadSales() {
    std::vector<SaleRecord> list;
    const char *sql = "SELECT drug_name, quantity, timestamp, operator FROM sales ORDER BY id ASC";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(static_cast<sqlite3*>(dbHandle), sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SaleRecord r;
        r.drugName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        r.quantity = sqlite3_column_int(stmt, 1);
        r.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.operatorName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        list.push_back(r);
    }
    sqlite3_finalize(stmt);
    return list;
}