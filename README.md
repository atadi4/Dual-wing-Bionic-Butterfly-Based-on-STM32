## 项目简介

基于 STM32F103C8T6 的双翅仿生蝴蝶飞控固件。工程由 STM32CubeMX 生成并使用 CMake 组织构建。

## 工程结构

- `Core/Inc`, `Core/Src`：应用代码（主控制、定时器、串口、GPIO、中断等）
- `Drivers/STM32F1xx_HAL_Driver`：HAL 驱动
- `Drivers/CMSIS`：CMSIS 设备与内核支持
- `cmake/`：工具链与 CubeMX 子工程 CMake 配置
- `STM32F103XX_FLASH.ld`：链接脚本
- `startup_stm32f103xb.s`：启动文件
- `test.ioc`：CubeMX 工程配置

## 核心功能

1. **舵机控制（扑翼输出）**
   - TIM1 CH1/CH2 输出 PWM，驱动左右两侧舵机。
2. **遥控输入解析**
   - TIM3 CH1 输入捕获解析 PPM 脉宽，将各通道值写入数组。
3. **扑翼运动控制**
   - 根据遥控器通道映射得到升降、频率、差速、幅度、零位微调参数；
   - 以余弦轨迹生成双舵机扑翼动作，支持静止姿态与扑翼两种状态切换。
4. **调试串口**
   - USART1（重映射 PB6/PB7）用于日志输出与调试。

## 编译配置与工具链

- 构建系统：CMake + Ninja（`CMakePresets.json`）
- 工具链：`cmake/starm-clang.cmake`
  - 编译器：`starm-clang` / `starm-clang++`
  - 目标：`cortex-m3`
  - 链接脚本：`STM32F103XX_FLASH.ld`
- 产物：
  - ELF：`build/Debug/test.elf`
  - HEX/BIN：构建后自动由 `objcopy` 生成

## 硬件接口

- MCU：STM32F103C8T6
- 舵机 PWM：
  - PA8 -> TIM1_CH1
  - PA9 -> TIM1_CH2
- PPM 输入：
  - PA6 -> TIM3_CH1
- 串口：
  - PB6 -> USART1_TX（重映射）
  - PB7 -> USART1_RX（重映射）

## 编译与下载

### 1) 编译

```bash
cmake --preset Debug
cmake --build --preset Debug
```

> 需要先安装并配置 `starm-clang` 工具链（保证命令在 PATH 中可用）。

### 2) 下载/调试

仓库提供 `openocd.cfg`（J-Link + STM32F1 target），可按本机 OpenOCD 环境执行下载。

---

> 说明：本项目使用 CubeMX 生成代码并保留用户区（`/* USER CODE BEGIN */`），后续建议仅在用户区内扩展业务逻辑，避免重新生成时被覆盖。
