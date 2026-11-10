# 分层与接缝：五层底座与主机清洁线

## 背景

固件在裸机 STM32G474 上分层 `board / drivers / osal / service / app`，并移植 Zephyr v4.5.0 的设备模型 + init + 日志框架（落于 `lib/` 与 `subsys/`）。软件侧要求零 STM32 头，以便在 Linux + GCC 主机上用 fake 驱动做单测。

## 决策

- **五层 + 底座**：`board / drivers / osal / service / app`；Zephyr 移植落于 `lib/` 与 `subsys/`（设备模型 + init + 日志，见 ADR-0005）；`third_party/` 含 CMSIS_6、STM32 HAL/LL。ecsp 协议库（计划中的第二底座）尚未接入。
- **主机清洁线 = `drivers/include` 的 Zephyr 公开驱动 API 头**（`gpio.h`、`clock_control.h`、`dma.h`、`flash.h`…）。此线以上（`osal/service/app`、`lib/` 的 Zephyr 移植、`drivers/include` 自身）禁含 STM32/MCU 头；此线以下（`drivers/<drv>/`、`board/`）可用。构建期由 `tools/check_no_stm32.py` 强制（当前扫描 `osal service app lib/utils`）。
- **组合根**：设备绑定当前在 `app/main.c` 的 `main()` 完成（`device_get_binding` + 组装 `app_config` 注入）；尚无独立 `board/devices.c`。原规则「app 不碰 `device_get_binding`、绑定收敛在 board 组合根」尚未在骨架落地，需后续确认是改决策还是补实现。
- service 之间零互调；能力/机制归 service，生命周期决策归 app。

## 理由

主机清洁线让 `osal/service/app`、Zephyr 移植与 `drivers/include` 在主机上直接编译运行；fake 驱动实现同一套 API、走同一套设备模型做「记录-回放」，测试打真实接口形状而非手写桩。service 零互调保证能力/机制归 service、生命周期决策归 app。

## 依据

- drivers 层复用 Zephyr 公开驱动 API 头，不另造平行实现（驱动实现复用见 ADR-0003）。
