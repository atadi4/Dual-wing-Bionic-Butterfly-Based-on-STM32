## Dual-wing-Bionic-Butterfly-Based-on-STM32

基于 **STM32F103C8T6** 的双翼仿生蝴蝶飞控程序。项目使用 STM32CubeMX 生成 HAL 工程，并在 `main.c` 中实现了：

- 双舵机扑翼余弦轨迹控制
- PPM 遥控信号解析（TIM3 输入捕获中断）
- 起飞/停机逻辑、升降与差速转向控制
- 通过 CMake + OpenOCD 的编译与烧录流程

---

## 1. 项目结构

- `Core/Src/main.c`：核心控制逻辑（PPM解析、通道映射、扑翼算法）
- `Core/Src/tim.c`：定时器配置
  - TIM1：PWM 输出（两路舵机）
  - TIM3：输入捕获（PPM）
- `Core/Src/usart.c`：串口1初始化（115200，可用于调试输出）
- `Core/Src/stm32f1xx_it.c`：中断入口
- `test.ioc`：CubeMX 工程配置
- `CMakeLists.txt` / `cmake/`：CMake 构建脚本
- `openocd.cfg`：OpenOCD 下载配置（J-Link + STM32F1）

---

## 2. 主要控制逻辑（`Core/Src/main.c`）

程序初始化后启动 TIM1 PWM 与 TIM3 输入捕获，主循环持续读取通道数据并计算两路舵机脉宽：

- **静止模式**（`fly = 0`）：仅根据升降与差速调整舵机角度
- **扑翼模式**（`fly = 1`）：按余弦函数驱动两侧舵机往复扑翼

关键参数：

- `fd`：扑翼幅度（由通道7三挡切换）
- `ys`：每步延时（决定扑翼频率）
- `sj`：升降修正
- `cs`：差速修正（转向）
- `servoC_0 / servoC_1`：两侧舵机中位（由通道5/6微调）

---

## 3. 遥控通道映射

代码中将 PPM 通道映射为以下控制量（输入范围约 `595~1595us`）：

- CH1 → `cs`：差速转向（`-150 ~ 150`）
- CH2 → `sj`：升降（`-100 ~ 100`，含死区）
- CH3 → `ys`：频率控制（映射为延时 `10000 ~ 8500us`）
- CH5 → `c5`：A0 舵机中位微调
- CH6 → `c6`：A1 舵机中位微调
- CH7 → `fd`：三挡幅度切换（约 400 / 450 / 500）

当 CH3 使 `ys < 9100` 时进入扑翼模式；`ys > 9000` 时退出扑翼。

---

## 4. 硬件与引脚

基于 `test.ioc` 当前配置：

- MCU：`STM32F103C8T6`
- 舵机PWM输出：
  - `PA8` → TIM1_CH1
  - `PA9` → TIM1_CH2
- PPM输入：
  - `PA6` → TIM3_CH1（上升沿捕获）
- 串口调试：
  - `PB6` → USART1_TX
  - `PB7` → USART1_RX
- 板载LED：
  - `PC13` 输出

---

## 5. 构建与烧录

### 5.1 环境依赖

- CMake (>= 3.22)
- Ninja
- ARM 交叉工具链（项目默认配置为 `starm-clang`）
- OpenOCD
- J-Link（对应 `openocd.cfg`）

> 注意：在当前环境直接执行 `cmake --preset Debug` 会因缺少 `starm-clang/starm-clang++` 而失败，请先安装并加入 PATH，或按需修改 `cmake/starm-clang.cmake`。

### 5.2 编译

```bash
cmake --preset Debug
cmake --build --preset Debug
```

编译成功后会在构建目录生成：

- `test.elf`
- `test.hex`
- `test.bin`

### 5.3 烧录（OpenOCD）

```bash
openocd -f openocd.cfg -c "program build/Debug/test.elf verify reset exit"
```

---

## 6. 开发说明

- 推荐通过 STM32CubeMX 打开 `test.ioc` 维护外设配置，并保留用户代码区（`USER CODE BEGIN/END`）。
- 目前项目核心控制代码集中在 `main.c`，后续可按“接收机解析 / 飞行状态机 / 姿态控制”进一步模块化拆分。
