# 部署文档

## 1. 项目概述

Plain 是一个基于 C++17 与 SFML 的 2D 顶视角游戏原型，项目当前依赖以下核心组件：

- C++17 编译器
- MinGW / GCC
- SFML 3.0.2
- Windows 平台运行环境

本项目使用 `makefile` 进行构建，输出为 Windows 可执行文件 `Plain.exe`。

---

## 2. 环境要求

### 2.1 必需软件

1. MinGW / GCC
   - 推荐使用 MinGW64
   - 例如：`D:\mingw64\bin\g++.exe`

2. SFML 3.0.2
   - 头文件路径：`D:/SFML-3.0.2/include`
   - 库文件路径：`D:/SFML-3.0.2/lib`

3. Windows 操作系统
   - 当前项目以 Windows 作为主运行环境

### 2.2 目录要求

项目根目录结构需要包含：

```text
plain/
├── src/
├── include/
├── texture/
├── fonts/
├── makefile
└── README.md
```

运行时还需要保证以下资源存在：

- `texture/player.png`
- `texture/ai.png`
- `texture/tree.png`
- `texture/stone.png`
- `fonts/AliPuHui.ttf`

否则程序在启动时会出现资源未加载的异常。

---

## 3. 构建方式

### 3.1 直接使用 makefile

在项目根目录执行：

```bash
make
```

构建过程会：

- 编译 `src/` 下所有 `.cpp` 文件
- 生成 `obj/` 临时对象文件
- 输出可执行文件 `Plain.exe`

### 3.2 Release 构建

```bash
make release
```

输出文件：

```text
Plain_release.exe
```

### 3.3 清理构建产物

```bash
make clean
```

---

## 4. 关键编译配置

当前 `makefile` 中的编译参数为：

```make
CXX        = g++
CXXFLAGS   = -Wall -Wextra -std=c++17 -g -Iinclude -ID:/SFML-3.0.2/include -MMD -MP
LDFLAGS    = -L D:/SFML-3.0.2/lib -mconsole
LDLIBS     = -lsfml-graphics -lsfml-window -lsfml-system
```

这意味着：

- 使用 C++17
- 打开额外警告
- 链接 SFML 图形、窗口和系统模块
- 使用控制台模式运行（适用于开发阶段调试）

---

## 5. 运行方式

### 5.1 本地运行

编译完成后，直接运行：

```bash
Plain.exe
```

### 5.2 运行时注意事项

程序运行时需要当前工作目录为项目根目录，否则相对路径会失效，例如：

- `./texture/player.png`
- `./fonts/AliPuHui.ttf`

如果从其他目录启动可执行文件，纹理和字体加载可能失败。

因此建议采用以下方式：

- 在项目根目录打开终端
- 再执行可执行文件

---

## 6. 常见部署问题

### 6.1 SFML 库找不到

表现：

- 程序启动失败
- 链接错误：无法找到 `sfml-graphics` 等库

处理方案：

- 检查 `D:/SFML-3.0.2/lib` 是否存在
- 检查 `g++` 能否正确找到该路径
- 确认 SFML 版本与项目兼容

### 6.2 资源文件加载失败

表现：

- 窗口打开，但角色/建筑/字体没有显示
- 控制台输出 `Failed to load font file.`

处理方案：

- 确认 `texture/` 和 `fonts/` 路径正确
- 确认文件名与代码一致
- 确认从项目根目录运行

### 6.3 资源路径依赖工作目录

当前代码中使用了相对路径，例如：

```cpp
"./texture/player.png"
"./fonts/AliPuHui.ttf"
```

如果程序在错误目录启动，资源会找不到。

建议的部署方式：

- 把资源目录复制到输出目录
- 或者将其改成基于可执行文件所在目录解析的路径

---

## 7. 推荐部署方式（稳定版）

为了保证项目能稳定运行，推荐采用下面的目录布局：

```text
Plain/
├── Plain.exe
├── texture/
│   ├── player.png
│   ├── ai.png
│   ├── tree.png
│   └── stone.png
├── fonts/
│   └── AliPuHui.ttf
└── dll/
    └── SFML runtime dlls
```

如果在 Windows 环境中直接发布，需要确保以下 SFML 运行时 DLL 在程序同目录下：

- `sfml-graphics-3.dll`
- `sfml-window-3.dll`
- `sfml-system-3.dll`

否则程序可能启动时提示缺少 DLL。

---

## 8. 发行建议

### 8.1 开发版

适合内部测试：

- 直接使用 `Plain.exe`
- 仍保留源码和资源目录

### 8.2 交付版

适合给测试人员使用：

- 打包 exe
- 打包 `texture/`
- 打包 `fonts/`
- 打包 SFML 运行时 DLL
- 统一放在同一目录

---

## 9. 结论

这个项目的部署方式本身并不复杂，但关键在于：

- 使用正确的 MinGW + SFML 环境
- 资源路径必须正确
- 程序需要从项目根目录或带资源目录的部署目录运行

只要满足这些条件，就可以稳定进行本地构建和运行。

---

## 10. 常用命令速查

```bash
make
make release
make clean
Plain.exe
```

如果你需要，我也可以继续补一份：

- Windows 发布包脚本版文档
- Linux/macOS 交叉编译说明
- CI/CD 自动构建说明
