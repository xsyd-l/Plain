#==============================================================================
# Makefile for Plain Game
#==============================================================================

# Use Windows command shell for compatibility with MinGW/mingw32-make on Windows
SHELL      = cmd.exe

#------------------------------------------------------------------------------
# 工具链与第三方库路径（均可在命令行覆盖，例如：make SFML_DIR=E:/SFML-3.0.2）
#------------------------------------------------------------------------------
CXX        = g++
SFML_DIR  ?= D:/SFML-3.0.2
SFML_INC  ?= $(SFML_DIR)/include
SFML_LIB  ?= $(SFML_DIR)/lib
SFML_BIN  ?= $(SFML_DIR)/bin

#------------------------------------------------------------------------------
# 编译 / 链接参数
#------------------------------------------------------------------------------
CXXFLAGS   = -Wall -Wextra -std=c++17 -g -Iinclude -I$(SFML_INC) -MMD -MP
LDFLAGS    = -L$(SFML_LIB) -mconsole
LDLIBS     = -lsfml-graphics -lsfml-window -lsfml-system

#------------------------------------------------------------------------------
# 项目配置
#------------------------------------------------------------------------------
TARGET_BASE = Plain
SRCDIR      = src
OBJDIR      = obj
DISTDIR     = dist

# 递归收集源文件：支持任意层级子目录（如 src/spark/Spark.cpp）
rwildcard   = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
SRCS        = $(sort $(call rwildcard,$(SRCDIR)/,*.cpp))
OBJS        = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS        = $(OBJS:.o=.d)
OBJDIRS     = $(sort $(dir $(OBJS)))

# 运行 exe 所需的 SFML 运行时 DLL（位于 $(SFML_BIN)）
# sfml-graphics-3.dll sfml-window-3.dll sfml-system-3.dll

# 构建模式: debug (默认) 或 release
BUILD       ?= debug

#==============================================================================
# 构建模式配置
#==============================================================================
ifeq ($(BUILD),release)
    CXXFLAGS += -O2 -DNDEBUG
    TARGET    = $(TARGET_BASE)_release
else
    CXXFLAGS += -O0 -DDEBUG
    TARGET    = $(TARGET_BASE)
endif

#==============================================================================
# PHONY 目标
#==============================================================================
.PHONY: all clean distclean rebuild release debug info dlls run dist help

# 默认目标
all: info $(TARGET)

# 链接
$(TARGET): $(OBJS) makefile
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo == Build complete: $@

# 创建目标目录
$(OBJDIRS):
	@if not exist "$@" mkdir "$@"

# 编译规则
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 包含依赖文件
-include $(DEPS)

# 辅助目标
info:
	@echo Build configuration:
	@echo   BUILD    = $(BUILD)
	@echo   TARGET   = $(TARGET)
	@echo   SFML_DIR = $(SFML_DIR)
	@echo   SRCS     = $(SRCS)
	@echo   OBJS     = $(OBJS)

# Debug 模式
debug:
	$(MAKE) all BUILD=debug

# Release 模式
release:
	$(MAKE) all BUILD=release

# 重新编译
rebuild: clean all

# 运行：构建后启动游戏，并把 SFML 的 bin 目录临时加入 PATH
# （程序用相对路径读 texture/ 、 fonts/ ，因此必须在项目根目录启动）
run: $(TARGET)
	@echo == Running: $(TARGET).exe
	@set "PATH=$(subst /,\,$(SFML_BIN));%PATH%" && $(TARGET).exe

# 把 SFML 运行时 DLL 复制到项目根目录（与 exe 同目录才能直接双击运行）
dlls:
	@copy /y "$(SFML_BIN)\sfml-graphics-3.dll" . >nul
	@copy /y "$(SFML_BIN)\sfml-window-3.dll" . >nul
	@copy /y "$(SFML_BIN)\sfml-system-3.dll" . >nul
	@echo == SFML runtime DLLs ready

# 生成可分发包 $(DISTDIR)/：exe + 资源 + SFML 运行时 DLL
dist: $(TARGET)
	@if not exist "$(DISTDIR)" mkdir "$(DISTDIR)"
	@copy /y "$(TARGET).exe" "$(DISTDIR)" >nul
	@if exist "texture" xcopy /e /i /y "texture" "$(DISTDIR)\texture" >nul
	@if exist "fonts" xcopy /e /i /y "fonts" "$(DISTDIR)\fonts" >nul
	@copy /y "$(SFML_BIN)\sfml-graphics-3.dll" "$(DISTDIR)" >nul
	@copy /y "$(SFML_BIN)\sfml-window-3.dll" "$(DISTDIR)" >nul
	@copy /y "$(SFML_BIN)\sfml-system-3.dll" "$(DISTDIR)" >nul
	@echo == Distribution ready: $(DISTDIR)/

# 清理：删除 obj/ 与可执行文件
clean:
	@if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	@if exist "$(TARGET_BASE).exe" del /f "$(TARGET_BASE).exe"
	@if exist "$(TARGET_BASE)_release.exe" del /f "$(TARGET_BASE)_release.exe"
	@echo == Clean complete

# 彻底清理：clean + 分发目录 + 复制进来的 DLL
distclean: clean
	@if exist "$(DISTDIR)" rmdir /s /q "$(DISTDIR)"
	@if exist "sfml-graphics-3.dll" del /f "sfml-graphics-3.dll"
	@if exist "sfml-window-3.dll" del /f "sfml-window-3.dll"
	@if exist "sfml-system-3.dll" del /f "sfml-system-3.dll"
	@echo == Distclean complete

# 帮助
help:
	@echo Plain build targets:
	@echo   make            debug build   -^> $(TARGET_BASE).exe
	@echo   make release    release build -^> $(TARGET_BASE)_release.exe
	@echo   make run        build and run (SFML bin added to PATH)
	@echo   make dlls       copy SFML runtime DLLs next to the exe
	@echo   make dist       create $(DISTDIR)/ with exe, assets and DLLs
	@echo   make clean      remove $(OBJDIR)/ and exe files
	@echo   make distclean  clean + remove $(DISTDIR)/ and copied DLLs
	@echo   make rebuild    clean + all
	@echo   make info       show current build configuration
	@echo Overridable vars: CXX, SFML_DIR, BUILD
