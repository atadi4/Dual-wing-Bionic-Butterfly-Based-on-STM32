# Dual-wing-Bionic-Butterfly-Based-on-STM32

基于 STM32F103 的双翼仿生蝴蝶控制工程，使用 CMake + Ninja 构建。

## 构建环境

- CMake 3.22+
- Ninja
- `starm-clang` 工具链（`starm-clang`、`starm-clang++`、`starm-objcopy` 需在 `PATH` 中）
- OpenOCD（用于下载）

> 当前工程默认使用 `cmake/starm-clang.cmake` 工具链配置。

## 编译

在仓库根目录执行：

```bash
cmake --preset Debug
cmake --build --preset Debug
```

编译产物位于：

- `build/Debug/test.elf`
- `build/Debug/test.hex`
- `build/Debug/test.bin`

## 烧录（OpenOCD）

仓库内提供了 `openocd.cfg`（J-Link + STM32F1），可直接使用：

```bash
openocd -f openocd.cfg -c "program build/Debug/test.elf verify reset exit"
```
