# 编译器只用 armclang（AC6）

移植 Zephyr 设备模型时，其工具链适配层只覆盖 armclang（`__ARMCOMPILER_VERSION`，即 AC6），ARMCC5（`__CC_ARM`）没有适配层。决定：本板固件一律用 armclang（AC6）编译，不用 ARMCC5。

理由：设备模型 / init / 日志依赖的 GNU 属性（`__attribute__((section/aligned/used/weak))`）在 armclang 下经 llvm→gcc 映射直接可用；ARMCC5 需另写适配，且已 EOL。详见 research 票 #2 的报告。

代价：启动文件、scatter 文件、内联汇编都要按 armclang 写，Keil 工程必须选 AC6；armlink 无 GNU ld `SORT` 等价物，init 段排序需自写等价方案（见 #2 报告「未确证」项）。
