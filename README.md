# 双翼仿生蝴蝶（基于 STM32）

基于 STM32F103C8T6的双翼仿生扑翼蝴蝶飞行器控制程序。通过遥控器 PPM 信号驱动两个舵机，用余弦曲线模拟蝴蝶翅膀的扑动，实现起飞、升降、转向等动作。

本项目由 Arduino 版本移植到 STM32 HAL 库，使用 STM32CubeMX 生成外设初始化代码，CMake + arm-none-eabi-gcc 工具链编译。

## 功能特性

- **扑翼运动**：将半个扑翼周期分成 18 份，按 `y = A·cos(ωx) + h` 余弦曲线控制舵机角度，左右翅膀对称扑动。
- **遥控控制**：解析 7 通道 PPM 信号，实时控制飞行姿态。
- **起飞保护**：通道 3 拨到最低时 `fly = 0`（停桨），推过阈值后 `fly = 1` 启动扑翼。
- **舵机微调**：通道 5 / 6 旋钮可在线微调两个舵机的中位，方便机械装配后校准。

## 硬件平台

| 项目 | 说明 |
|------|------|
| 主控 | STM32F103C8T6（STM32F1 系列，72 MHz） |
| 执行机构 | 2 × 舵机（500–2500 µs 脉宽，1500 µs 中位） |
| 输入 | 遥控接收机 PPM 信号 |
| 时钟 | HSI + PLL ×4 |

## 引脚分配

| 功能 | 定时器 / 外设 | 引脚 |
|------|--------------|------|
| 舵机 A0（PWM 输出） | TIM1_CH1 | PA8 |
| 舵机 A1（PWM 输出） | TIM1_CH2 | PA9 |
| PPM 输入捕获 | TIM3_CH1 | PA6 |
| 串口调试 TX | USART1_TX | PB6 |
| 串口调试 RX | USART1_RX | PB7 |

- **TIM1**：预分频 71、周期 19999 → 50 Hz PWM（20 ms 周期），匹配标准舵机。
- **TIM3**：输入捕获模式解析 PPM，脉宽 > 5000 视为帧同步间隔，复位通道计数。
- **USART1**：115200 波特率，已重定向 `_write`，可用 `printf` 调试（代码中默认注释）。

## 遥控通道定义

| 通道 | 作用 | 映射范围 |
|------|------|----------|
| CH1 | 差速（控制转向，左右翅幅度差） | -150 ~ 150 |
| CH2 | 升降（静止时控制倾角） | -100 ~ 100 |
| CH3 | 扑翼频率 / 起飞开关 | 延时 10000 ~ 8500 µs |
| CH5 | 舵机 A0 中位微调 | 0 ~ 200 |
| CH6 | 舵机 A1 中位微调 | 0 ~ 200 |
| CH7 | 扑翼幅度三档切换 | 400 / 450 / 500 |

> 通道死区：CH1、CH2 映射值绝对值小于 5 时归零，避免摇杆中位抖动。

## 工程结构

```
Core/Src/main.c     主程序：PPM 解析、扑翼算法、舵机控制
Core/Src/tim.c      TIM1（舵机 PWM）、TIM3（PPM 捕获）配置
Core/Src/usart.c    串口调试配置
Core/Src/gpio.c     GPIO 初始化
test.ioc            STM32CubeMX 工程文件
CMakeLists.txt      CMake 构建配置
openocd.cfg         OpenOCD 下载配置
```

## 编译与下载

使用 GCC + CMake 工具链，OpenOCD 烧录。

### 依赖

- [arm-none-eabi-gcc](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) 工具链
- [CMake](https://cmake.org/) 3.22+
- [OpenOCD](https://openocd.org/)
- ST-Link 调试器

### 编译

```bash
cmake --preset Debug
cmake --build build/Debug
```

### 下载到芯片

```bash
openocd -f openocd.cfg -c "program build/Debug/test.elf verify reset exit"
```

## 核心算法说明

扑翼通过余弦插值实现，把半周期分成 18 个角度点（0°~170°）：

```c
pulsewidth_0 = (servoC_0 - sj) - (fd + cs) * cos((10 * i) / 180.0 * 3.14);
pulsewidth_1 = (servoC_1 + sj) + (fd - cs) * cos((10 * i) / 180.0 * 3.14);
```

- `fd`（幅度）控制余弦曲线振幅，即翅膀扑动幅度。
- `servoC`（中位）+ `sj`（升降）控制曲线零点位置。
- `cs`（差速）让左右翅膀幅度不对称，从而实现转向。
- `ys`（延时）控制每个角度点之间的间隔，间隔越短扑翼频率越高。

## 许可

外设代码版权归 STMicroelectronics，遵循其 LICENSE。
