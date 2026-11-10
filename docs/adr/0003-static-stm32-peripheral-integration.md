# 以静态板级配置复用 STM32 外设驱动

状态：记录已确认的设计方向；DMA 映射与部分 NVIC 细节仍待核实。

## 背景

本项目复用从 Zephyr 移植来的 STM32 GPIO、pinctrl、UART 与 DMA 驱动，但不完整移植 Zephyr 的设备树、clock-control、runtime-PM 和 IRQ 连接机制。板级硬件和 USART1/USART2 的协议口选择均已知，可以由 board 层静态描述。

## 决策

### 通用决策

- 所有设备驱动均移除对设备树生成实例的依赖，不使用设备树生成实例；由 board 提供静态实例及其板级配置。
- 不移植完整 Zephyr 设备树、clock-control 与 runtime-PM 机制；固定板级资源由 board 和各驱动初始化路径显式配置。
- 复用现有驱动及其 API，不为同一硬件能力另建平行实现。

### 时钟与电源管理

- 不使用 runtime-PM；GPIO、UART 和相关路径不依赖运行时电源管理。
- `SystemClock_Config()` 负责系统与总线时钟配置。
- 外设在各自初始化时直接开启所需外设时钟，不移植完整 clock-control 子系统。

### GPIO 与 pinctrl

- 保留现有 GPIO API 和驱动，不另建一套板级 GPIO 控制器。
- GPIO 静态端口实例由 board 提供，并仅声明已分配的引脚掩码。
- pinctrl 采用无设备树的最小静态配置，仅需支持 DEFAULT 状态，并复用现有 pinctrl/GPIO 配置机制。
- 静态端口覆盖 GPIOA、GPIOB、GPIOD、GPIOG；USART1 被编译选中时额外包含 GPIOC。每个端口仅声明已分配的引脚。

### GPIO 中断

- 保留现有 GPIO callback 语义及 EXTI trampoline/callback 机制。
- 用静态 IRQ 映射替代设备树生成的 `IRQ_CONNECT`。
- 启动向量由强定义的 EXTI handler 静态接管并转发至 trampoline；注入式 `irq_config_func` 只配置 NVIC 优先级和使能，不动态安装 ISR。
- 安全相关出波/过流 EXTI 的优先级高于 UART 与 DMA；EXTI 分组可分别配置优先级。

### UART 与 DMA

- 复用现有 `uart_stm32.c` 的 UART API 和硬件操作；UART 静态实例由 board 按通用静态实例化决策提供。
- USART1 与 USART2 继续由编译期配置二选一，运行时不切换。
- 保留 UART DMA 和硬件流控代码路径。选中的协议 UART 必须使用 DMA，RX 与 TX 均启用；DMA controller、channel 和 request 由 board 静态配置，并复用现有 DMA 抽象/驱动。
- 当前板级流控配置为关闭；保留流控能力不代表当前实例启用 RTS/CTS。当前接口需求规定无流控，且未配置 RTS/CTS 引脚。
- UART IRQ 使用静态向量 handler 转发到现有 UART ISR，并由注入式 `irq_config_func` 配置 NVIC。
- UART 与 DMA IRQ 处于同一优先级，且低于安全相关 EXTI。具体数值在确认 NVIC priority grouping 后确定。

## 理由

静态 board 配置与当前固定硬件相符，可以复用已有驱动及其 API，同时避免引入本项目并不需要的完整 Zephyr 设备树、clock-control 与 runtime-PM 机制。明确静态 IRQ 接线和优先级关系，也使安全相关中断不会被通信或 DMA 抢占。

## 待核实事项

- 核实 STM32G474 上所选 USART 的 RX/TX DMA controller、channel 与 request 映射，再落入 board 配置；不得猜测或沿用未经确认的设备树值。
- 确认 NVIC priority grouping，并据此确定 EXTI、UART、DMA 的具体优先级数值。
- 确认系统时钟配置失败时的 fail-closed 路径能够阻止后续业务初始化。

## 依据

- [需求规格说明书](../需求规格说明书.md)：协议口选择、串口格式、无流控要求及安全相关中断优先级约束。
- [MCU引脚功能映射表](../MCU引脚功能映射表.md)：板级 GPIO/USART 引脚分配及系统时钟目标。
