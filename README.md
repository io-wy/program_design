# 药房销售系统（命令行版 / C++17）

## Readme是ai乱写的

## 功能概述
- 药品数据管理：新增、查询（按名称/分类）、修改、删除
- 库存与销售：模拟销售（库存不足提示）、累计销售量维护
- 临期药品：按配置的保质期与临期阈值，显示临期清单
- 报表：销售统计（总销量、全量排行、总库存）
- 数据持久化：SQLite 数据库（`data/pharmacy.db`），配置读取（`config.txt`）

## 数据库结构（SQLite）
- 表 `drugs`：`name, category, manufacturer, specification, production_date, stock, total_sold, shelf_life_days, near_expiry_days`
- 表 `users`：`username, password, role`
- 表 `sales`：`id, drug_name, quantity, timestamp, operator`
  - `production_date` 格式：`YYYY-MM-DD`
  - 默认管理员账号：`admin/admin`

## 构建与运行（CMake）
```bash
# Windows PowerShell（需安装 CMake 与 C++编译器，如 MSVC 或 MinGW）
cd f:\code\others\program_design
mkdir build
cd build
cmake .. -G "Ninja"   # 或者使用 "MinGW Makefiles" / "NMake Makefiles" / Visual Studio 生成器
cmake --build .
./pharmacy_cli.exe     # 运行（不同生成器可能有不同产物路径）
```

- 若没有 Ninja，可直接用 `cmake ..` 默认生成器，再 `cmake --build .`
- MinGW 用户可用 `-G "MinGW Makefiles"` 构建。

## 运行说明
- 程序启动会初始化并连接 `data/pharmacy.db`
- 所有操作通过菜单进行；支持多次操作后再退出
- 退出前可手动保存，或在退出时选择保存

## 目录结构
```
program_design/
  ├─ CMakeLists.txt
  ├─ README.md
  ├─ require.md
  ├─ config.txt
  ├─ data/
  │   └─ drugs.csv
  └─ src/
      └─ main.cpp
```

## 备注
- 为满足“临期药品”功能，系统采用“生产日期 + 保质期（天）”计算有效期；
  - 分类保质期优先于默认保质期；未配置分类时使用默认值。
  - 临期判断：距离有效期剩余天数 ≤ `near_expiry_threshold_days`。
- 可根据实际需要调整配置文件中的保质期与阈值。
