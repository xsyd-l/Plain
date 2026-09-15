#==============================================================================
# Makefile for Plain Game
#==============================================================================

# Use Windows command shell for compatibility with MinGW/mingw32-make on Windows
SHELL      = cmd.exe

# 编译器
CXX        = g++
CXXFLAGS   = -Wall -Wextra -std=c++17 -g -Iinclude -ID:/SFML-3.0.2/include -MMD -MP
LDFLAGS    = -L D:/SFML-3.0.2/lib -mconsole
LDLIBS     = -lsfml-graphics -lsfml-window -lsfml-system

# 项目配置
TARGET_BASE = Plain
SRCDIR      = src
OBJDIR      = obj

# 自动收集源文件（支持一级子目录）
SRCS        = $(sort $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/*/*.cpp))
OBJS        = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS        = $(OBJS:.o=.d)
OBJDIRS     = $(sort $(dir $(OBJS)))

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
.PHONY: all clean rebuild release debug info

# 默认目标
all: info $(TARGET)

# 链接
$(TARGET): $(OBJS)
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

# 清理
clean:
	@if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	@if exist "$(TARGET_BASE).exe" del /f "$(TARGET_BASE).exe"
	@if exist "$(TARGET_BASE)_release.exe" del /f "$(TARGET_BASE)_release.exe"
	@echo == Clean complete
