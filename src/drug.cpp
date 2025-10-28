#include "drug.h"
#include <iomanip>
#include <sstream>

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