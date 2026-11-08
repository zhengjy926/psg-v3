# Build the STM32G474 target firmware with armclang (AC6) + armlink + scatter.
# Usage (from repo root):  powershell -File tools\build_target.ps1
#
# Requires Keil MDK-ARM with Arm Compiler for Embedded 6 (armclang/armlink).
# Set $env:ARMCLANG_BIN to override the default toolchain path.

$root = Split-Path -Parent $PSScriptRoot
$toolchain = if ($env:ARMCLANG_BIN) { $env:ARMCLANG_BIN } else { 'D:\Keil_v5\ARM\ARMCLANG\bin' }
$acc = Join-Path $toolchain 'armclang.exe'
$al  = Join-Path $toolchain 'armlink.exe'
$fr  = Join-Path $toolchain 'fromelf.exe'

foreach ($t in @($acc, $al, $fr)) {
    if (-not (Test-Path $t)) { throw "Toolchain binary not found: $t (set `$env:ARMCLANG_BIN)" }
}

$out = Join-Path $root 'build\target'
New-Item -ItemType Directory -Force -Path $out | Out-Null

$cflags = @(
    '--target=arm-arm-none-eabi', '-mcpu=cortex-m4', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16',
    '-std=c11', '-Wall', '-Wextra',
    '-I', (Join-Path $root 'vendored\zephyr\include'),
    '-I', $root
)

$srcs = @(
    'vendored\zephyr\src\init.c',
    'vendored\zephyr\src\device.c',
    'vendored\zephyr\src\logging.c',
    'vendored\zephyr\src\assert.c',
    'osal\osal.c',
    'service\heartbeat\heartbeat.c',
    'app\app.c',
    'drivers\clock_control\clock_stm32.c',
    'drivers\gpio\gpio_stm32.c',
    'board\board.c',
    'board\devices.c',
    'board\stm32g474\system_stm32g474.c',
    'board\stm32g474\main.c'
)
$asm = 'board\stm32g474\startup_stm32g474.s'
$scatter = Join-Path $root 'board\stm32g474\stm32g474.sct'

$objs = @()
foreach ($s in $srcs) {
    $o = Join-Path $out ([IO.Path]::GetFileNameWithoutExtension($s) + '.o')
    & $acc @cflags -c (Join-Path $root $s) -o $o 2>$null
    if ($LASTEXITCODE -ne 0) { throw "armclang failed: $s" }
    $objs += $o
}
$startup_obj = Join-Path $out 'startup_stm32g474.o'
& $acc @cflags -c (Join-Path $root $asm) -o $startup_obj 2>$null
if ($LASTEXITCODE -ne 0) { throw "armclang assemble failed: $asm" }
$objs += $startup_obj

$axf = Join-Path $out 'psg.axf'
$mapOut = & $al --scatter $scatter --map -o $axf $objs 2>$null
if ($LASTEXITCODE -ne 0) { throw 'armlink failed' }
$mapOut | Set-Content -Encoding ascii (Join-Path $out 'map.txt')

& $fr --bin $axf --output (Join-Path $out 'psg.bin') 2>$null
if ($LASTEXITCODE -ne 0) { throw 'fromelf failed' }

# Verify the init-section ordering (level + priority).
& python (Join-Path $root 'tools\verify_init_order.py') (Join-Path $out 'map.txt')
if ($LASTEXITCODE -ne 0) { throw 'init order verification failed' }

Write-Host "Target build OK: $axf"
