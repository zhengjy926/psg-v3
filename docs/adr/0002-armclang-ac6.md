# 编译器只用 armclang（AC6）

移植 Zephyr 设备模型时，其工具链适配层只覆盖 armclang（`__ARMCOMPILER_VERSION`，即 AC6），ARMCC5（`__CC_ARM`）没有适配层。决定：本板固件一律用 armclang（AC6）编译，不用 ARMCC5。

代价：启动文件、scatter 文件、内联汇编都要按 armclang 写，Keil 工程必须选 AC6；armlink 无 GNU ld `SORT` 等价物，init 段排序需自写等价方案（排序方案见 ADR-0005）。
