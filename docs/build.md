# 构建与测试（T01 骨架）

本仓有两套可编译工程 + CI（issue #21）：

| 工程 | 工具链 | 产出 | 验证 |
| --- | --- | --- | --- |
| 目标固件 | armclang AC6 (Keil MDK) + armlink + scatter | `build/target/psg.axf` / `psg.bin` | 编译通过 + 生成 init 排序表 |
| 主机测试 | Linux + GCC + Unity + ASan/UBSan | `build/host/host_tests` | 单测通过 + service/app/osal 不含 STM32 头（构建期强制） |

## 环境要求

- **目标固件**：Windows + Keil MDK（armclang AC6 / armlink），ARMCC5 不支持（ADR-0002）。
- **主机测试**：Linux（GCC + libasan/libubsan）。Windows 机器上经 **WSL** 跑，CI 用 `ubuntu-latest`。
- **原生 Windows 不支持主机测试**：mingw64 缺 libasan/libubsan，且其 PE 链接器不认 `tests/host/init_sections.ld` 的 `INSERT AFTER .rodata`（ELF 特性），即使 `-DENABLE_SANITIZERS=OFF` 也链接出坏 exe。需要原生 Windows 主机测试时，把主机端边界符号换成构建期生成的排序表（研究票 #2 的「路径 B」）。

## 目录

```
lib/  subsys/      移植 Zephyr v4.5.0 设备模型 + init + 日志最小闭包（Apache-2.0）
third_party/unity/ Unity 测试框架（MIT）
board/             目标板支持（STM32G474：startup/scatter/board init，可含 STM32 头）
drivers/include/   驱动 ops 接口（Zephyr 公开 API 头，主机清洁接缝，禁含 STM32 头）
drivers/<drv>/     目标驱动（含 STM32 头）
osal/  service/  app/   主机清洁层（禁含 STM32 头）
tests/host/        主机单测 + fake 驱动（第二 adapter）
tools/             构建脚本、静态检查、init 顺序校验
```

## 目标固件（Windows + Keil MDK，armclang AC6）

```powershell
powershell -File tools\build_target.ps1
```

脚本用 armclang 编译、armlink 按 `board/stm32g474.sct` 链接，并跑
`tools/verify_init_order.py` 校验 init 段按 (level, prio) 单调排序。默认工具链
`D:\Keil_v5\ARM\ARMCLANG\bin`，可用 `$env:ARMCLANG_BIN` 覆盖。

**init 排序契约**：init 顺序由构建期生成的排序表驱动，不依赖链接器排序。
`tools/generate_init_order.py` 从 `.o` 提取 `__init_*` 符号与段名里的
(level, prio, sub)，按数字排序后生成 `lib/utils/init_order.inc`；`init_runner.c`
遍历该表。scatter 只把 `.z_init_*` 全部 KEEP 进单个 `ER_INIT` region。
新增/调整 `SYS_INIT`/`DEVICE_DEFINE` 时无需改 scatter——重跑 `build_target.ps1`
即重生成排序表，并把更新的 `init_order.inc` 提交（Keil 编译的是入库这份）。

**Keil uVision 工程（并行构建，必须同时通过）**：目标固件改动除脚本构建外，还必须在
`project/MDK-ARM/PFA_PSG.uvprojx` 中用命令构建并通过：

```powershell
& 'D:\Keil_v5\UV4\UV4.exe' -r 'project\MDK-ARM\PFA_PSG.uvprojx' -j0 -o 'project\MDK-ARM\Objects\build.log'
```

要求 `0 Error(s)`（`L6314W` 空 init level 属预期）；产物落 `project/MDK-ARM/Objects/`。
`.uvoptx` / `JLinkSettings.ini` 是本机文件、不入库；缺失时 uVision 用 `.uvprojx` 的配置构建。

## 主机测试（Linux + GCC + Unity + ASan/UBSan）

```sh
cmake -S . -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host      # 同时跑 check_no_stm32 硬约束
ctest --test-dir build/host --output-on-failure
```

`check_no_stm32` 在每次构建时扫描 `osal/ service/ app/ lib/utils/`，含 STM32 头
即构建失败（硬约束）。ASan/UBSan 默认开启，`-DENABLE_SANITIZERS=OFF` 可关
（见上方「环境要求」：主机测试须在 Linux/WSL 下构建）。

host 侧用 `tests/host/autoconf_host.h`（`-imacros` 注入，定义 `CONFIG_X86` 等
host 专属 CONFIG），并消费入库的 `lib/utils/init_order_host.inc`。该文件由
`tools/generate_init_order.py readelf ...` 生成；CI 有一步重生成 + diff，改动
host 的 `SYS_INIT`/`DEVICE_DEFINE` 后须在 Linux/WSL 重生成该文件并提交。

## 静态检查（clang-tidy + cppcheck + MISRA）

```sh
tools/static_analysis.sh build/host /usr/share/cppcheck/addons
```

- clang-tidy：`.clang-tidy`（clang-analyzer/CERT/bugprone），analyzer 命中即失败。
- cppcheck 内置：`--enable=warning,error --error-exitcode=1`。
- cppcheck MISRA addon：`tools/misra/misra_2012_rules.txt` + 偏差表
  `tools/misra/README.md`，未屏蔽违规即失败。

## 关键验收对照

- AC#1 目标固件**双构建**：`build_target.ps1`（脚本 / CI）**且** Keil uVision 工程 `project/MDK-ARM/PFA_PSG.uvprojx` 的 `UV4 -r` 均 `0 Error(s)`；init 排序表由 `generate_init_order.py` 生成。
- AC#2 主机工程编译 + 无 STM32 头 → CMake 主机构建 + `check_no_stm32`。
- AC#3 clang-tidy + cppcheck(MISRA) → `static_analysis.sh` + `.github/workflows/ci.yml`。
