# 运行方法（Windows / MinGW）

本项目为 C++17 命令行程序，使用 CMake 构建。以下步骤基于 MinGW 环境，同时给出其他常用生成器的替代说明。这是傻逼ai文档

## 1. 环境准备
- 安装 CMake（3.10+）
- 安装 C++ 编译器（MinGW 或 MSVC）
- 推荐在 PowerShell 运行命令

## 2. 构建（MinGW Makefiles）
```powershell
cd f:\code\others\program_design
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```
- 构建完成后会生成 `pharmacy_cli.exe`（位于 `build/` 目录）

（可选）其他生成器：
- Ninja：`cmake .. -G "Ninja"`
- Visual Studio：`cmake .. -G "Visual Studio 17 2022"`

## 3. 运行
建议在 PowerShell 切换到 UTF-8，以避免中文乱码：
```powershell
chcp 65001
```
启动程序（从项目根目录或 `build` 目录均可）：
```powershell
# 方式一：在项目根目录运行构建产物
.\build\pharmacy_cli.exe

# 方式二：在 build 目录内运行
cd .\build
.\pharmacy_cli.exe
```
程序会读取以下文件：
- 配置：`config.txt`
- 数据：`data/drugs.csv`

注：程序内部已设置控制台为 UTF-8，通常无需每次执行 `chcp 65001`，但不同终端/字体可能仍需。

## 4. 菜单与操作
常用操作：新增、查询（名称/分类）、修改、删除、显示临期、模拟销售、报表、保存、重新载入、退出。
- 退出时可选择是否保存数据到 `data/drugs.csv`。

## 5. 脚本化测试（批量输入）
管道重定向：
```powershell
Get-Content .\test_input1.txt | .\build\pharmacy_cli.exe
Get-Content .\test_input2.txt | .\build\pharmacy_cli.exe
```
示例文件已提供：
- `test_input1.txt`
- `test_input2.txt`

## 6. 数据文件与配置
- 数据表头：`name,category,production_date,stock,total_sold`
- 日期格式：`YYYY-MM-DD`
- 配置键：
  - `default_shelf_life_days`（默认保质期，天）
  - `near_expiry_threshold_days`（临期阈值，天）
  - `shelf_life.<分类>=<天数>`（分类保质期优先）

## 7. 常见问题排查
- 中文显示为方块：更换终端字体为支持中文的字体（如微软雅黑/宋体）。
- 相对路径读取失败：请在项目根或 `build` 目录运行；确保存在 `config.txt` 和 `data/` 目录。
- 未找到数据文件：程序首次运行会提示，将在保存时创建；也可手动在 `data/` 下提供 `drugs.csv`。