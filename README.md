# stm32f429bit6_demo

基于 STM32CubeMX 的 STM32F429BIT6 裸机工程，使用 CMake + GCC 构建，包含 USART1 串口回显测试，以及 OpenOCD 烧录/调试配置。

## 硬件

| 项目 | 说明 |
|------|------|
| MCU | STM32F429BIT6（Cortex-M4，LQFP208） |
| 调试器 | ST-Link（SWD） |
| 串口 | USART1，PA9 = TX，PA10 = RX，115200 8N1 |

### ST-Link 接线

| ST-Link | 板子 |
|---------|------|
| SWDIO | SWDIO |
| SWCLK | SWCLK |
| GND | GND |
| 3.3V | 3.3V（如需要） |

### 串口接线

USB 转串口与 MCU 交叉连接：PC TX → PA10(RX)，PC RX → PA9(TX)。

## 软件依赖

| 工具 | 用途 | 下载 |
|------|------|------|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) | 交叉编译 (`arm-none-eabi-gcc`) | 必选 |
| [CMake](https://cmake.org/download/) | 构建系统 | 必选 |
| MinGW `mingw32-make` 或 [Ninja](https://ninja-build.org/) | 构建后端 | 二选一 |
| [xPack OpenOCD](https://github.com/xpack-dev-tools/openocd-xpack/releases) | 烧录 | 烧录时需要 |
| [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) | VS Code / Cursor 调试扩展 | 调试时需要 |

## 快速开始

### 1. 克隆仓库

```bash
git clone git@github.com:Biao-qing/stm32f429bit6_demo.git
cd stm32f429bit6_demo
```

### 2. 配置工具链路径

编辑 `build.ps1` 和 `flash.ps1` 中的 `$ToolchainBin` / `$OpenOcd`，改为你本机的安装路径，例如：

```powershell
$ToolchainBin = "C:\Program Files\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin"
$OpenOcd = "C:\Program Files\xpack-openocd-0.12.0-7\bin\openocd.exe"
```

调试时还需修改 `.vscode/settings.json` 中的 `cortex-debug.armToolchainPath` 和 `cortex-debug.openocdPath`。

建议将工具链和 OpenOCD 的 `bin` 目录加入系统 PATH。

### 3. 编译

```powershell
.\build.ps1
```

或双击 `build.bat`，或在 Cursor / VS Code 中按 `Ctrl+Shift+B`。

编译产物：`build\Debug\stm32f429bit6_demo.elf`

### 4. 烧录

先编译，再烧录（`flash.ps1` 不会自动编译）：

```powershell
.\build.ps1
.\flash.ps1
```

或在 Cursor / VS Code 中：`Ctrl+Shift+P` → `Tasks: Run Task` → **Flash**

### 5. 串口测试

1. 打开串口助手，115200 8N1
2. 复位板子，应看到欢迎信息
3. 输入任意字符，MCU 会原样回显
4. 每 5 秒输出一次 `[heartbeat]`

## 脚本说明

### build.ps1

| 命令 | 说明 |
|------|------|
| `.\build.ps1` | Debug 编译 |
| `.\build.ps1 -Config Release` | Release 编译 |
| `.\build.ps1 -Clean` | 仅删除 `build\<Config>`，不编译 |

### flash.ps1

| 命令 | 说明 |
|------|------|
| `.\flash.ps1` | 烧录 Debug 固件 |
| `.\flash.ps1 -Config Release` | 烧录 Release 固件 |

若未编译就烧录，会提示先执行 `build.ps1`。

## 调试

1. 安装 **Cortex-Debug** 扩展
2. 连接 ST-Link
3. `Ctrl+Shift+D` 打开调试面板，选择 **OpenOCD + ST-Link**
4. 按 **F5** 开始调试（会自动编译并烧录，停在 `main`）

常用快捷键：F5 继续，F10 单步跳过，F11 单步进入，Shift+F5 停止。

## 手动 CMake 编译

未安装 Ninja 时：

```powershell
$env:PATH = "C:\Program Files\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin;" + $env:PATH

cmake -B build/Debug -DCMAKE_BUILD_TYPE=Debug `
  "-DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake" `
  -G "MinGW Makefiles"

cmake --build build/Debug
```

已安装 Ninja 时可直接使用：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

## 项目结构

```
├── Core/                  # 应用代码（main、外设、中断）
├── Drivers/               # CMSIS + STM32 HAL
├── cmake/                 # 工具链与 CubeMX 生成的 CMake 配置
├── .vscode/               # 编译任务、OpenOCD 调试配置
├── build.ps1 / flash.ps1  # 编译与烧录脚本
├── openocd.cfg            # OpenOCD 配置（ST-Link + STM32F4）
├── stm32f429bit6_demo.ioc # STM32CubeMX 工程
└── CMakeLists.txt
```

## 外设配置

- **USART1**：115200 8N1，DMA 已在 CubeMX 中配置（当前代码使用中断接收）
- **系统时钟**：180 MHz（HSI → PLL）

修改外设请用 STM32CubeMX 打开 `.ioc` 重新生成，用户代码写在 `/* USER CODE BEGIN */` 区域内以免被覆盖。

## 常见问题

**编译报错找不到 `arm-none-eabi-gcc`**  
检查 `build.ps1` 中 `$ToolchainBin` 路径，或把工具链 `bin` 加入 PATH。

**烧录连不上芯片**  
检查 ST-Link 接线；关闭占用 ST-Link 的其他软件（如 STM32CubeIDE）；可在 `openocd.cfg` 中降低 `adapter speed`。

**串口无输出**  
确认波特率 115200，TX/RX 交叉连接，共地。

**F5 调试无反应**  
确认已安装 Cortex-Debug，并在 `.vscode/settings.json` 中配置正确的 OpenOCD 和工具链路径。

## 许可证

STM32 HAL / CMSIS 遵循 ST 许可条款，见 `Drivers/` 下各 `LICENSE.txt`。
