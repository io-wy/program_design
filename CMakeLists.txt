cmake_minimum_required(VERSION 3.10)
# project(pharmacy_cli LANGUAGES CXX)
project(pharmacy_cli LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pharmacy_cli 
    src/main.cpp
    src/drug.cpp
    src/pharmacy.cpp
    src/database.cpp
)

# -------------------------------
# 构建 SQLite 动态库 (来自源码)
# -------------------------------
set(SQLITE_DIR "${CMAKE_SOURCE_DIR}/third_party/sqlite")

# 构建 sqlite3 静态库（更简单、避免导出符号问题）
add_library(sqlite3 STATIC
    ${SQLITE_DIR}/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC ${SQLITE_DIR})

# 使用 C99，并关闭扩展加载（更安全）
target_compile_features(sqlite3 PRIVATE c_std_99)
target_compile_definitions(sqlite3 PRIVATE SQLITE_THREADSAFE=1 SQLITE_OMIT_LOAD_EXTENSION)

# -------------------------------
# 链接到你的主程序
# -------------------------------
target_sources(pharmacy_cli PRIVATE src/sqlite_db.cpp)
target_link_libraries(pharmacy_cli PRIVATE sqlite3)
target_compile_definitions(pharmacy_cli PRIVATE HAS_SQLITE=1)

message(STATUS "✅ SQLite3 built from source and linked dynamically")