# MISRA C:2012 静态检查与偏差表

本仓用 **cppcheck（MISRA addon）+ clang-tidy** 做静态门禁（issue #21 验收项 3）。
MISRA C:2012 规则原文不可再分发，本目录的 `misra_2012_rules.txt` 只含公开的
「附录 A」规则号 + 类别（Required/Advisory/Mandatory）+ 一句话标题；取得授权后
可用官方文本替换。重新生成：`python tools/misra/gen_misra_rules.py`。

## 运行

```sh
tools/static_analysis.sh build/host /usr/share/cppcheck/addons
```

- 参数 1：CMake 构建目录（含 `compile_commands.json`）。
- 参数 2：cppcheck 的 `addons/` 目录（`misra.py` 所在处）。

## 偏差表（suppressed rules）

以下规则因领域原因偏离，已通过 `--suppress-rules` 屏蔽并在代码注释中说明。
**Required/Mandatory 规则之外的其余规则不允许偏离**（Advisory 8.9/11.5/15.5
为记录的工程取舍）。

| 规则 | 类别 | 偏离理由 |
| --- | --- | --- |
| 8.9 | Advisory | service 状态（心跳配置、os 滴答计数）须静态存储期，跨多次调用存活。 |
| 8.11 | Advisory | 设备注册表等 iterable 段是链接器拼接的数组，大小由链接器决定，无法写显式大小。 |
| 11.3 | Required | iterable 段/链接符号边界把 `char *` 强转成 `const struct device *`（`Image$$…$$Base/Limit`、`_device_list_*`），是「散落段当连续数组走」的设备模型机制。 |
| 11.5 | Advisory | 设备模型把不透明 `const void *api` 强转成类型化 API 结构（`DEVICE_API_GET`），是 Zephyr 设备模型的既定契约。 |
| 15.5 | Advisory | 入口守卫用早返回保持函数平铺、避免深层嵌套。 |
| 17.1 | Required | 日志格式化（`z_vformat`/`z_log_printf`）用 `<stdarg.h>` 变参实现 printf 风格输出，是 MINIMAL 日志路径的固有形态。 |
| 17.8 | Required | 极简格式化器用 `char **`/`size_t *` 出参推进缓冲位置，是自写 `vsnprintf` 的惯用形态。 |
| 20.7 / 20.10 / 21.1 | Required | Zephyr 移植（`lib/utils` / `subsys/logging`）的宏 DSL（`toolchain.h`/`util_macro.h`/`iterable_sections.h`/`init.h`/`log.h`）依赖 `#`/`##` 与保留标识符，是移植 Zephyr 设备模型/init/日志的固有形态。 |
| 18.2 / 18.3 | Required | iterable 段遍历对同一链接数组的 `_list_start`/`_list_end` 做指针关系比较，是「把散落段当连续数组走」的设备模型机制（cppcheck 内置 `comparePointers`，经 `--suppress=comparePointers` 屏蔽）。 |

> 说明：`18.2/18.3` 由 cppcheck 内置检查报为 `comparePointers`（非 `misra-c2012-*`），
> 在脚本里以 `--suppress=comparePointers` 屏蔽；其余在 misra addon 里以
> `--suppress-rules=8.9,8.11,11.3,11.5,15.5,17.1,17.8,20.7,20.10,21.1` 屏蔽。
