#include "drug.h"
#include <iomanip>
#include <sstream>
#include <cctype>

// 解析严格格式 YYYY-MM-DD 到 std::tm（必须为四位年、两位月、两位日，且日期有效）
bool parseDate(const std::string &dateStr, std::tm &tmOut) {
    if (dateStr.size() != 10) return false;
    // 检查形态 YYYY-MM-DD
    for (int i = 0; i < 4; ++i) if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) return false;
    if (dateStr[4] != '-') return false;
    for (int i = 5; i < 7; ++i) if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) return false;
    if (dateStr[7] != '-') return false;
    for (int i = 8; i < 10; ++i) if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) return false;

    auto toInt2 = [](char a, char b) { return (a - '0') * 10 + (b - '0'); };
    int year = (dateStr[0]-'0')*1000 + (dateStr[1]-'0')*100 + (dateStr[2]-'0')*10 + (dateStr[3]-'0');
    int month = toInt2(dateStr[5], dateStr[6]);
    int day = toInt2(dateStr[8], dateStr[9]);

    if (month < 1 || month > 12) return false;
    if (day < 1) return false;

    // 计算每月最大天数，考虑闰年
    auto isLeap = [](int y){ return (y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0)); };
    int maxDay;
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: maxDay = 31; break;
        case 4: case 6: case 9: case 11: maxDay = 30; break;
        case 2: maxDay = isLeap(year) ? 29 : 28; break;
        default: maxDay = 31; // 不会到这里
    }
    if (day > maxDay) return false;

    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0; tm.tm_min = 0; tm.tm_sec = 0;
    tm.tm_isdst = -1;
    tmOut = tm;
    return true;
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