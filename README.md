# stm32f429bit6_demo

STM32F429BIT6 嵌入式 Demo：CMake + GCC + FreeRTOS + W25Q64 + FlashDB + 双路 CAN（J1939/UDS）+ 以太网（LwIP），分层工程结构。

## 硬件

| 项目 | 说明 |
|------|------|
| MCU | STM32F429BIT6（Cortex-M4，LQFP208） |
| 调试器 | ST-Link（SWD，PA13/PA14） |
| 串口 | USART1，PA9=TX / PA10=RX，115200 8N1 |
| LED | PD12（1Hz）、PG7（4Hz） |
| SPI Flash | W25Q64，SPI3（PB3/PB4/PB5），CS=PG3 |
| 以太网 | LAN8720 PHY，RMII |
| CAN1 | PB8/PB9，500 kbit/s，J1939 SA=0x28 |
| CAN2 | PB12/PB13，500 kbit/s，J1939 SA=0x29 |

### ST-Link 接线

| ST-Link | 板子 |
|---------|------|
| SWDIO | SWDIO |
| SWCLK | SWCLK |
| GND | GND |
| 3.3V | 3.3V（可选） |

串口：PC TX → PA10，PC RX → PA9，共地。

## 软件依赖

| 工具 | 用途 |
|------|------|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) | `arm-none-eabi-gcc` |
| [CMake](https://cmake.org/download/) | 构建 |
| MinGW `mingw32-make` 或 [Ninja](https://ninja-build.org/) | 构建后端 |
| [xPack OpenOCD](https://github.com/xpack-dev-tools/openocd-xpack/releases) | 烧录/调试 |
| [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) | VS Code / Cursor 调试 |
| [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) | GCC IntelliSense（可选） |

## 快速开始

### 1. 克隆并拉取第三方库

```bash
git clone git@github.com:Biao-qing/stm32f429bit6_demo.git
cd stm32f429bit6_demo
.\setup-thirdparty.ps1
```

`setup-thirdparty.ps1` 会拉取 FlashDB、Open-SAE-J1939、isotp-c（LwIP 与 FreeRTOS 已随仓库入库）。  
首次编译时若缺少依赖，`build.ps1` 也会自动调用该脚本。

### 2. 配置工具链

编辑 `build.ps1`、`flash.ps1` 中的路径，或加入 PATH：

```powershell
$ToolchainBin = "C:\Program Files\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin"
```

### 3. 编译

```powershell
.\build.ps1
```

产物：`build\Debug\stm32f429bit6_demo.elf`  
同时生成 `compile_commands.json` 供 IDE 索引。

### 4. 烧录

```powershell
.\build.ps1
.\flash.ps1
```

### 5. 串口 CLI

上电后串口 115200，可用命令（无需回车，停打约 400ms 自动执行）：

| 命令 | 说明 |
|------|------|
| `help` | 帮助 |
| `log` | 查看最新日志 |
| `log all` | 按时间正序导出 |
| `log cnt` | 日志条数 |

FlashDB 在后台初始化，首次启动可能需等待数秒后再执行 `log`。

## 项目结构（分层）

```
Application/     L5  应用任务（LED、CLI、启动编排）
Services/        L4  FlashDB / 网络 / CAN 服务
Platform/        L3  FAL、LwIP、CAN-IF、J1939/UDS 适配与用户回调
Device/          L2  W25Q64、LAN8720 驱动
Core/            L1  CubeMX 生成（勿手改）
Drivers/         L0  CMSIS + HAL
ThirdParty/FreeRTOS/          FreeRTOS 内核（入库）
ThirdParty/LwIP/              LwIP（入库，源自 STM32CubeF4 V1.28.3）
ThirdParty/FlashDB/           setup-thirdparty.ps1 拉取，不入库
ThirdParty/Open-SAE-J1939/    setup-thirdparty.ps1 拉取，不入库
ThirdParty/isotp-c/           setup-thirdparty.ps1 拉取，不入库
cmake/                        分层 CMake 目标
```

### CAN 用户代码（手改区）

| 文件 | 说明 |
|------|------|
| `Platform/Src/can_user_j1939.c` | J1939 ECU 填充、PGN 接收、周期发送 |
| `Platform/Inc/can_user_j1939_msg.h` | PGN/信号矩阵与解码 |
| `Platform/Src/can_user_uds.c` | UDS 配置与请求处理 |

UDS 诊断 ID：RX `0x7E0`/`0x7DF`，TX `0x7E8`。

## FreeRTOS 任务

| 任务 | 优先级 | 说明 |
|------|--------|------|
| `led1` / `led2` | +2 | LED 闪烁 |
| `uart_cmd` | +2 | 串口 CLI |
| `flash_init` | +1 | FlashDB 初始化（完成后自删除） |
| `can_boot` | +1 | CAN-IF / J1939 / UDS 初始化 |
| `j1939_*` / `uds_*` | +1 | 双路 CAN 协议栈任务 |

- 内核：FreeRTOS V10.6.2，`heap_4`，堆 **24 KB**
- HAL 时基：TIM6；SysTick 留给 FreeRTOS
- 入口：`main()` → `App_FreeRTOS_Init()`

## IDE 索引（GCC）

1. 安装 **Microsoft C/C++** 扩展（`ms-vscode.cpptools`）
2. 运行 `.\build.ps1` 生成 `compile_commands.json`
3. `Ctrl+Shift+P` → `Developer: Reload Window`
4. 配置见 `.vscode/c_cpp_properties.json`（`gcc-arm` + `arm-none-eabi-gcc`）

## CubeMX 重新生成

用 STM32CubeMX 打开 `stm32f429bit6_demo.ioc` 生成代码。  
用户代码写在 `/* USER CODE BEGIN */` 区域内。  
`Application/`、`Device/`、`Platform/`、`Services/` 不受 CubeMX 影响。

## 调试

1. 安装 Cortex-Debug
2. `Ctrl+Shift+D` → **OpenOCD + ST-Link** → F5

## 常见问题

**找不到 `arm-none-eabi-gcc`**  
检查 `build.ps1` 中 `$ToolchainBin` 或 PATH。

**编译报 FlashDB 相关错误**  
执行 `.\setup-thirdparty.ps1` 拉取缺失的第三方库。

**`log` 提示 FlashDB not ready**  
等待 `[flashdb] init ok` 出现后再试，或查看 W25Q64 接线。

**烧录失败**  
检查 ST-Link 接线，关闭占用调试器的其他程序。

## 许可证

STM32 HAL / CMSIS 遵循 ST 许可；FlashDB 见 [armink/FlashDB](https://github.com/armink/FlashDB)；Open-SAE-J1939 见 [DanielMartensson/Open-SAE-J1939](https://github.com/DanielMartensson/Open-SAE-J1939)；isotp-c 见 [SimonCahill/isotp-c](https://github.com/SimonCahill/isotp-c)。
