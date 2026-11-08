# 构建与测试（T01 骨架）

本仓有两套可编译工程 + CI（issue #21）：

| 工程 | 工具链 | 产出 | 验证 |
| --- | --- | --- | --- |
| 目标固件 | armclang AC6 (Keil MDK) + armlink + scatter | `build/target/psg.axf` / `psg.bin` | 编译通过 + init 段按 level+prio 排序 |
| 主机测试 | Linux + GCC + Unity + ASan/UBSan | `build/host/host_tests` | 单测通过 + service/app/osal 不含 STM32 头（构建期强制） |

## 目录

```
vendored/zephyr/   移植 Zephyr v4.5.0 设备模型 + init + 日志最小闭包（Apache-2.0）
vendored/unity/    Unity 测试框架（MIT）
board/             目标板支持（STM32G474：startup/scatter/system/组合根，可含 STM32 头）
drivers/api/       驱动 ops 接口（主机清洁接缝，禁含 STM32 头）
drivers/<drv>/     目标驱动（含 STM32 头）
osal/  service/  app/   主机清洁层（禁含 STM32 头）
tests/host/        主机单测 + fake 驱动（第二 adapter）
tools/             构建脚本、静态检查、init 顺序校验
```

## 目标固件（Windows + Keil MDK，armclang AC6）

```powershell
powershell -File tools\build_target.ps1
```

脚本用 armclang 编译、armlink 按 `board/stm32g474/stm32g474.sct` 链接，并跑
`tools/verify_init_order.py` 校验 init 段按 (level, prio) 单调排序。默认工具链
`D:\Keil_v5\ARM\ARMCLANG\bin`，可用 `$env:ARMCLANG_BIN` 覆盖。

**scatter 排序契约**：每个 init level 占一个执行区（`ER_INIT_<LEVEL>`），区内
`.z_init_<LEVEL>_P_<prio>_SUB_<sub>_` 段名零填充 3 位（`P_001`/`P_010`/…），
armlink 按段名字典序放置 → 字典序=数值序。已实证（研究票 #2 的「未确证」项）。

## 主机测试（Linux + GCC + Unity + ASan/UBSan）

```sh
cmake -S . -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host      # 同时跑 check_no_stm32 硬约束
ctest --test-dir build/host --output-on-failure
```

`check_no_stm32` 在每次构建时扫描 `osal/ service/ app/ drivers/api/`，含 STM32 头
即构建失败（硬约束）。ASan/UBSan 默认开启，`-DENABLE_SANITIZERS=OFF` 可关
（部分 Windows GCC 无 libasan）。

## 静态检查（clang-tidy + cppcheck + MISRA）

```sh
tools/static_analysis.sh build/host /usr/share/cppcheck/addons
```

- clang-tidy：`.clang-tidy`（clang-analyzer/CERT/bugprone），analyzer 命中即失败。
- cppcheck 内置：`--enable=warning,error --error-exitcode=1`。
- cppcheck MISRA addon：`tools/misra/misra_2012_rules.txt` + 偏差表
  `tools/misra/README.md`，未屏蔽违规即失败。

## 关键验收对照

- AC#1 目标工程 armclang 编译 + scatter 排序 → `build_target.ps1` + `verify_init_order.py`。
- AC#2 主机工程编译 + 无 STM32 头 → CMake 主机构建 + `check_no_stm32`。
- AC#3 clang-tidy + cppcheck(MISRA) → `static_analysis.sh` + `.github/workflows/ci.yml`。
