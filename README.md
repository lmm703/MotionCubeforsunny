# MotionCube

基于 Seeed Studio XIAO ESP32S3 与 MPU6050 的最小可运行 Arduino 固件。

这一轮实现目标：

- 使用 `Wire.begin()` 走开发板默认 I2C 引脚
- 启动时执行 I2C 扫描并打印发现到的设备地址
- 读取并打印 MPU6050 的 `WHO_AM_I`
- 初始化成功后每 50ms 输出一行传感器 CSV
- 新增基于加速度计和陀螺仪的轻量姿态输出
- 新增 BLE 姿态广播与 Android 网页立方体演示
- 不依赖第三方 MPU6050 库
- 不实现 DMP

## 文件说明

- `MotionCube.ino`：程序入口，负责串口、I2C 扫描、初始化与周期输出
- `mpu6050.h` / `mpu6050.cpp`：基础寄存器级 MPU6050 驱动
- `platformio.ini`：PlatformIO 兼容配置，复用同一套源码
- `web-demo/index.html`：Android Chrome 演示页入口
- `web-demo/styles.css`：网页样式与 3D 立方体外观
- `web-demo/app.js`：Web Bluetooth 连接与姿态数据显示逻辑

## 硬件连接

- XIAO ESP32S3 `3V3` -> MPU6050 `VCC`
- XIAO ESP32S3 `GND` -> MPU6050 `GND`
- XIAO ESP32S3 默认 `SDA` -> MPU6050 `SDA`
- XIAO ESP32S3 默认 `SCL` -> MPU6050 `SCL`
- MPU6050 `AD0` -> `GND`
- MPU6050 `INT` 暂不连接

## 串口输出格式

启动时会先输出 I2C 扫描结果与 `WHO_AM_I`，随后打印 CSV 头：

```text
millis,ax,ay,az,gx,gy,gz,temp
```

之后每 50ms 输出一行，例如：

```text
1250,312,-144,16408,-22,17,5,28.47
```

其中：

- `ax/ay/az` 为原始加速度计寄存器值
- `gx/gy/gz` 为原始陀螺仪寄存器值
- `temp` 为根据原始温度寄存器换算得到的摄氏度

## 姿态输出模式

`MotionCube.ino` 中保留了两个编译期开关：

```cpp
constexpr bool kEnableRawCsvOutput = false;
constexpr bool kEnableAttitudeCsvOutput = true;
```

默认只输出姿态 CSV：

```text
millis,roll,pitch
```

如果需要回到原始数据调试模式，只需把 `kEnableRawCsvOutput` 改为 `true`，并按需要决定是否同时保留姿态输出。

## BLE 快速演示模式

当前固件默认还会启用 BLE 姿态发送：

```cpp
constexpr bool kEnableBleAttitudeOutput = true;
```

启动后会以 `MotionCube` 为设备名进行广播，并通过自定义服务发送 `roll,pitch` 文本。

BLE 传输特性：

- 设备名：`MotionCube`
- 发送周期：约 `50 ms`
- 载荷格式：`roll,pitch`
- 示例：`-4.32,-52.18`

这样做的目的是让手机网页端最容易调试与解析，先快速打通演示链路。

## 互补滤波公式与参数

这版姿态输出只实现 `roll` 和 `pitch`，不实现 DMP，也不实现 `yaw`。

### 1. 加速度计倾角

先把原始加速度换算为 `g`：

```text
ax_g = ax / 16384
ay_g = ay / 16384
az_g = az / 16384
```

在当前 `ACCEL_CONFIG = 0x00` 的配置下，MPU6050 工作在 `+-2g`，灵敏度约为 `16384 LSB/g`。

倾角计算公式：

```text
rollAcc  = atan2(ay_g, az_g) * 180 / PI
pitchAcc = atan2(-ax_g, sqrt(ay_g^2 + az_g^2)) * 180 / PI
```

### 2. 陀螺仪积分

当前 `GYRO_CONFIG = 0x00`，量程为 `+-250 deg/s`，灵敏度约为 `131 LSB/(deg/s)`：

```text
gx_dps = gx / 131
gy_dps = gy / 131
```

积分更新：

```text
rollGyro  = roll  + gx_dps * dt
pitchGyro = pitch + gy_dps * dt
```

其中 `dt` 由相邻两次输出的 `millis()` 差值计算，单位为秒。

### 3. 互补滤波

最终姿态使用互补滤波融合：

```text
roll  = alpha * rollGyro  + (1 - alpha) * rollAcc
pitch = alpha * pitchGyro + (1 - alpha) * pitchAcc
```

当前默认参数：

```text
alpha = 0.98
sample interval = 50 ms
```

含义是：

- 陀螺仪负责短时动态响应
- 加速度计负责长期纠正漂移
- `alpha` 越大，输出越平滑、越依赖陀螺仪
- `alpha` 越小，输出越容易被线性加速度干扰，但长期更容易回正

## 当前限制

- 本版只输出 `roll` 和 `pitch`
- 不输出 `yaw`
- 原因是只有加速度计和陀螺仪时，`yaw` 缺少重力参考，纯陀螺积分会逐渐漂移
- 如果后续要让手机上的立方体实现更完整、更稳定的 3D 跟随旋转，通常还需要磁力计或更完整的姿态融合方案

## Android 网页立方体演示

`web-demo` 目录下提供了一个最小静态网页，可以直接用 Chrome on Android 连接 XIAO 的 BLE 姿态数据并显示立方体旋转效果。

### 为什么需要 HTTPS

Web Bluetooth 需要安全上下文，最稳妥的方式是通过 `https://` 页面访问演示页。

这意味着：

- 不建议直接双击本地 `index.html`
- 更适合把 `web-demo` 放到静态 HTTPS 托管上
- 例如 GitHub Pages、Gitee Pages 或任意 HTTPS 静态站点

### 手机端使用步骤

1. 上传当前固件到 XIAO ESP32S3。
2. 确认串口启动信息中显示 BLE 已启用，并看到设备名 `MotionCube`。
3. 将 `web-demo` 目录部署到 HTTPS 静态站点。
4. 在华为 Mate 60 Pro 上使用 Chrome 打开该 HTTPS 页面。
5. 点击 `Connect BLE`。
6. 在系统蓝牙选择框中选择 `MotionCube`。
7. 连接成功后，页面会显示实时 `roll`、`pitch` 数值，并让立方体随开发板倾斜而转动。

### 调试建议

- 如果网页能打开但无法连接 BLE，先确认浏览器是 Chrome，并且页面来自 HTTPS
- 如果能连接但立方体不动，先查看串口是否仍在输出姿态数据
- 如果旋转方向和你的直觉相反，优先在 `web-demo/app.js` 中调整前端映射方向，而不是先改传感器算法

## Arduino IDE 使用

1. 安装 ESP32 开发板支持包。
2. 用 Arduino IDE 打开 `MotionCube.ino` 所在文件夹。
3. 选择开发板 `Seeed XIAO ESP32S3`。
4. 选择正确串口。
5. 编译并上传。
6. 打开串口监视器，波特率设为 `115200`。

如果初始化失败，程序会输出明确错误并停止运行，方便优先排查供电、接线与 I2C 地址问题。
如果姿态模式开启，轻微倾斜开发板时，你应当看到 `roll` 与 `pitch` 持续变化并在静止后逐步稳定。
如果 BLE 模式开启，启动日志还会额外显示当前 BLE 广播状态。

## PlatformIO 使用

如果你更习惯 PlatformIO，可直接在当前目录执行：

```bash
pio run
pio device monitor -b 115200
```

`platformio.ini` 已配置 `src_dir = .`，因此无需调整当前 Arduino IDE 风格的目录结构。
