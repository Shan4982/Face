# 基于 iPPG 技术的非接触式生理信号与人脸监测系统

## 项目简介
本项目是一个基于**成像光电容积描记术 (Imaging Photoplethysmography, iPPG)** 的非接触式生命体征监测与人脸追踪系统。
该系统通过普通摄像头捕捉面部微血管血液容积的周期性变化，利用 OpenCV 与数字信号处理算法，在本地终端实现实时的高精度心率 (Heart Rate) 估计与活体状态监测。

项目采用**边缘计算架构与本地多线程模型**，通过 `OpenMP` 和 `OpenCL` 实现了极高的数据流处理性能与超低延迟，可广泛应用于智能座舱 (DMS)、智慧医疗监护、高安全级别安防门禁等场景。

---

## 核心特性 (Features)
- ?? **非接触式多模态监测**：无需佩戴任何设备，仅凭视频流即可实现人脸追踪与生理信号提取。
- ? **极致性能优化**：
  - **OpenCL (T-API) 硬件加速**：将图像预处理卸载至 GPU 运行。
  - **OpenMP 多线程并行**：在 CPU 层面加速 FFT (快速傅里叶变换) 频谱分析。
  - **零内存分配机制**：基于 `std::vector` 的环形缓冲区滑动窗口架构，彻底解决动态分配导致的内存碎片。
- ? **零拷贝线程通信**：设计 `ThreadChannel` 模板类，利用 `std::shared_ptr` 实现数据跨线程的零拷贝无锁传输，并集成防死锁与积压丢帧机制。
- ?? **高鲁棒性抗干扰**：
  - 集成基于拉普拉斯方差的**模糊检测剔除算法**，自动丢弃运动伪影帧。
  - 内置**生理信号模拟器**，在未检测到人脸时自动降级输出测试信号（含基线漂移与高斯噪声），保证系统不中断。

---

## 系统架构与模块分工

系统主要由以下三大核心模块构成：

### 1. 人脸检测与预处理模块 (Face Detection)
- **技术栈**: OpenCV, Haar/LBP 级联分类器
- **功能**:
  - 双模自适应人脸检测，提供稳定的 ROI 提取。
  - 动态降采样策略，保障检测性能，降低主线程开销。
  - 运动模糊检测，确保送入后续管道的图像质量。

### 2. iPPG 信号处理模块 (Signal Processing)
- **技术栈**: FFTW/OpenCV DFT, 数字信号滤波
- **功能**:
  - 从面部 ROI 提取绿通道 (Green Channel) 均值作为原始信号。
  - 去直流分量、运动伪影限幅与汉明窗 (Hamming Window) 频谱抗泄露处理。
  - 提取频域峰值，完成高精度的实时心率 (BPM) 换算。
  - ROI 一致性方差质量评估。

### 3. 系统集成与并发控制模块 (System Controller)
- **技术栈**: C++11 `std::thread`, `std::mutex`, `std::condition_variable`
- **功能**:
  - 实现视频采集与信号计算的完全解耦。
  - 在主界面实时叠加 UI：视频流追踪框、心率值、FPS、信号质量。

---

## 编译与运行指南 (Getting Started)

### 环境依赖
- C++ 11 或更高版本
- CMake 3.10+
- **OpenCV 4.x** (包含 `core`, `imgproc`, `highgui`, `videoio`, `objdetect` 模块)
- **OpenMP** (通常由 GCC/Clang/MSVC 编译器自带)
- OpenCL 驱动 (可选，用于启用 T-API 加速)

### 本地编译 (Linux / Windows)
```bash
# 1. 创建构建目录
mkdir build && cd build

# 2. 生成构建文件
cmake ..

# 3. 编译项目 (Windows 用户可使用 cmake --build . --config Release)
cmake --build . --config Release

# 4. 运行可执行文件
./Release/test.exe
```

---

## 未来展望 (Future Work)
- [ ] 重新集成基于 `opencv_contrib` 的 LBPH / CNN 人脸身份识别模型。
- [ ] 引入 CHROM 或 POS 颜色空间正交算法，进一步提升在复杂光照变化下的信号抗干扰能力。
- [ ] 支持血氧饱和度 (SpO2) 与呼吸率 (RR) 的多维度生理指标估算。
