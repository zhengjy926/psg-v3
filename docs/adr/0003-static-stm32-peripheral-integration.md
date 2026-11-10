# 以静态板级配置复用 STM32 外设驱动

状态：记录已确认的设计方向；DMA 映射、NVIC priority grouping、过流检出的中断/轮询选择与时钟 fail-closed 路径仍待核实。

## 背景

本项目复用从 Zephyr 移植来的 STM32 GPIO、pinctrl、UART 与 DMA 驱动，但不完整移植 Zephyr 的设备树、clock-control、runtime-PM 机制；中断连接只移植 IRQ_CONNECT/软件中断表的最小 API 面，不移植其背后的调度器与上下文切换机制。板级硬件和 USART1/USART2 的协议口选择均已知，可以由 board 层静态描述。

## 决策

### 通用决策

- 所有设备驱动均移除对设备树生成实例的依赖，不使用设备树生成实例；由 board 提供静态实例及其板级配置（设备模型整体无设备树见 ADR-0005）。
- 不移植完整 Zephyr 设备树、clock-control 与 runtime-PM 机制；固定板级资源由 board 和各驱动初始化路径显式配置。
- 复用 Zephyr 驱动及其 API，不为同一硬件能力另建平行实现。

### 时钟与电源管理

- 不使用 runtime-PM；GPIO、UART 和相关路径不依赖运行时电源管理。
- `SystemClock_Config()` 负责系统与总线时钟配置。
- 外设在各自初始化时直接开启所需外设时钟，不移植完整 clock-control 子系统。

### GPIO 与 pinctrl

- 保留现有 GPIO API 和驱动，不另建一套板级 GPIO 控制器。
- GPIO 静态端口实例由 board 提供，并仅声明已分配的引脚掩码。
- pinctrl 采用无设备树的最小静态配置，仅需支持 DEFAULT 状态，并复用现有 pinctrl/GPIO 配置机制。
- 静态端口覆盖 GPIOB、GPIOD、GPIOG；USART2 被编译选中时包含 GPIOA（PA2/PA3），USART1 被编译选中时包含 GPIOC（PC4/PC5）。每个端口仅声明已分配的引脚。

### GPIO 中断

- 保留现有 GPIO callback 语义及 EXTI trampoline/callback 机制（Zephyr 的两层结构：`intc_exti_stm32.c` 只操作 EXTI 寄存器，`intc_gpio_stm32.c` 提供 line→IRQ 表、callback 表与 trampoline）。
- 保留驱动侧 `IRQ_CONNECT`/`irq_enable` 调用点，仅把其中的设备树参数换成 board 静态常量；`line → NVIC IRQn` 与 `line → 向量分组` 由 board 静态表提供（见「中断控制器与 IRQ 接线」）。
- 启动向量不是「一个 EXTI handler」：STM32G4 上 EXTI 是多个向量（EXTI0–4 各一个、EXTI9_5、EXTI15_10），这些向量统一指向软件中断表分派器 `_isr_wrapper`（见「中断控制器与 IRQ 接线」）。
- 安全相关出波/过流 EXTI 的优先级高于 UART 与 DMA；NVIC 优先级只到向量粒度，同一向量内的 line（如 EXTI9_5 中的 R 波与过流锁存）靠 trampoline 分派顺序表达，过流锁存必须先于 R 波（N-004）。

### UART 与 DMA

- 复用现有 `uart_stm32.c` 的 UART API 和硬件操作；UART 静态实例由 board 按通用静态实例化决策提供。
- USART1 与 USART2 继续由编译期配置二选一，运行时不切换。
- 保留 UART DMA 和硬件流控代码路径。选中的协议 UART 必须使用 DMA，RX 与 TX 均启用；DMA controller、channel 和 request 由 board 静态配置，并复用现有 DMA 抽象/驱动。
- 当前板级流控配置为关闭；保留流控能力不代表当前实例启用 RTS/CTS。当前接口需求规定无流控，且未配置 RTS/CTS 引脚。
- UART IRQ 经 `_sw_isr_table` 转发到现有 UART ISR，并由注入式 `irq_config_func` 配置 NVIC。
- UART 与 DMA IRQ 处于同一优先级，且低于安全相关 EXTI。具体数值在确认 NVIC priority grouping 后确定。

### 中断控制器与 IRQ 接线

本决策横跨 EXTI、UART、DMA 与 COMP 四个中断源，单独成节。当前 [`board/startup_stm32g474.s`](../../board/startup_stm32g474.s) 是裸 CMSIS 向量表：16 个系统向量 + `.rept 100` 的 `.word Default_Handler`，device IRQ 槽位全指向 `Default_Handler`，没有 Zephyr 软件中断表。因此启动向量表必须改造后才能接任何中断。

- 移植 IRQ 连接机制的**最小 API 面**：`IRQ_CONNECT`（向 RAM 表注册）+ `irq_enable`/`irq_disable`（NVIC）+ `_sw_isr_table[]`（固定长度 RAM/BSS 表，按 IRQn 下标索引）+ 一个读 IPSR 的分派器 `_isr_wrapper`。不移植其背后的调度器、上下文切换与 `_IntExit`（本项目无线程，见 `osal.h`），也不用 `gen_isr_tables.py` 构建期生成器（表为初始化时运行时注册，无需生成器或 GNU ld `SORT`）。
- 分派器为汇编尾调用形态：读 `IPSR` → 减 16 得 IRQ 号 → 查 `_sw_isr_table[irqn]` → 未接线落到 `Default_Handler`，已接线以 `bx` 尾调用，使 ISR 的返回即中断返回（EXC_RETURN）。不得用普通 C `bl isr` 再返回。
- 优先级接线必须落到 `NVIC_SetPriority`，且对齐上游 `ARCH_IRQ_CONNECT` 的时机：`IRQ_CONNECT` 在驱动的 `irq_config_func` 中把 `{isr, arg}` 注册进 `_sw_isr_table[irqn]`（RAM）并立即 `NVIC_SetPriority(irqn, priority)`；`irq_enable` 只做 `NVIC_EnableIRQ(irqn)`，不设优先级。若漏掉优先级写入，中断默认 priority 0（最高）会破坏 N-004；须加断言/自检。
- 启动向量表必须改造：把 `.rept 100 .word Default_Handler` 换成与 G4 IRQn 表严格一致的 device IRQ 段、每个条目指向 `_isr_wrapper`；长度与顺序须含 85 号缺口、到 FMAC=101，不得沿用 `.rept`。
- `_sw_isr_table` 表项与上游 `struct _isr_table_entry` 同构：`{arg, isr}`（arg 在前，兼容上游 `ldmia` 一次装载），不含 priority；priority 在 `IRQ_CONNECT` 时直接写入 NVIC。`_sw_isr_table` 即静态 IRQ 映射表，作为 N-004 的代码审查凭证。
- 保留 Zephyr 的两层 EXTI 结构：`intc_exti_stm32.c` 只操作 EXTI 寄存器，`intc_gpio_stm32.c` 提供 line→IRQ 表、callback 表与 trampoline；只把其中的设备树来源换成 board 静态常量。驱动侧的 `IRQ_CONNECT`/`irq_enable` 调用点保留，仅把 `DT_INST_IRQN`/`DT_INST_IRQ(priority)` 换成 board 常量。
- STM32G4 的 EXTI 向量分组是硬件事实（EXTI0–4 各一个向量、EXTI9_5、EXTI15_10）。本板映射：

  - `EXTI4_IRQn`（line 4）：PB4 端口启停输入。
  - `EXTI9_5_IRQn`（line 5–9）：PB6 R 波、PG7/PG8 过流锁存（正/负）。
  - `EXTI15_10_IRQn`（line 10–15）：本板未用。

- NVIC 优先级只到向量粒度：EXTI4 与 EXTI9_5 是两个独立向量、各自一个优先级；EXTI9_5 内部无优先级，过流锁存（line 7/8）必须先于 R 波（line 6）分派。trampoline 不得沿用 Zephyr 按行号升序分派（那会把 R 波排到过流锁存前面），须按优先级顺序显式检查 line。出波/过流相关中断优先级高于 UART 与 DMA（N-004）。
- 范围说明：「出波相关中断」在本 ADR 内指 EXTI 的 R 波触发（PB6）与过流锁存（PG7/PG8）；出波本身的 HRTIM 中断（PA8/PA9 脉冲输出）不在本 ADR 的 GPIO/pinctrl/UART/DMA 范围，其优先级约束由 HRTIM 相关决策另行记录。
- 过流检出与过流锁存是两条独立中断路径：过流锁存走 EXTI（PG7/PG8），过流检出走比较器 `COMP1_2_3_IRQn = 64`（独立向量，COMP1/COMP3）。须确认过流检出走中断还是轮询：若走中断，其优先级同样受 N-004 约束并纳入 `_sw_isr_table`；若为轮询，明确「过流检出为轮询、不占中断」，N-004 的过流中断仅指 EXTI 过流锁存。

## 理由

静态 board 配置与当前固定硬件相符，可以复用已有驱动及其 API，同时避免引入本项目并不需要的完整 Zephyr 设备树、clock-control 与 runtime-PM 机制。明确静态 IRQ 接线和优先级关系，也使安全相关中断不会被通信或 DMA 抢占。

## 待核实事项

- 核实 STM32G474 上所选 USART 的 RX/TX DMA controller、channel 与 request 映射，再落入 board 配置；不得猜测或沿用未经确认的设备树值。
- 确认 NVIC priority grouping，并据此确定 EXTI、UART、DMA 的具体优先级数值。
- 确认端口启停输入（PB4/EXTI4）相对 UART 与 DMA 的优先级：是否与过流锁存/R 波同处高优先级组，还是另行定级。
- 确认过流检出（COMP1/COMP3）走中断还是轮询，据此决定 `COMP1_2_3_IRQn` 是否纳入静态 IRQ 映射表与 N-004 优先级约束。
- 确认系统时钟配置失败时的 fail-closed 路径能够阻止后续业务初始化。

## 依据

- [需求规格说明书](../需求规格说明书.md)：协议口选择、串口格式、无流控要求及安全相关中断优先级约束。
- [MCU引脚功能映射表](../MCU引脚功能映射表.md)：板级 GPIO/USART 引脚分配及系统时钟目标。
- [`stm32g474xx.h`](../../third_party/stm32/cmsis-device-g4/Include/stm32g474xx.h) 与 STM32G4 参考手册（RM0440）：EXTI 向量分组、IRQn 编号、COMP/DMA 映射等硬件事实。
