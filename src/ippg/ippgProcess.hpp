#pragma once
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>
#include <omp.h> // 引入OpenMP

using namespace std;
using namespace cv;

// 生理信号模拟发生器
class SignalSimulator {
private:
    double t = 0.0;
    double dt = 1.0 / 15.0; // 15fps
    double true_hr = 75.0;  // 模拟心率 75 BPM

public:
    // 生成包含基线漂移、高斯白噪声和运动伪影的模拟信号
    double generateSample();
};

class ippgProcess {
public:
    double get_RawSimple(const Mat& frame);
    // 信号质量评估策略：基于ROI一致性（例如计算绿通道方差）
    double evaluateSignalQuality(const Mat& roi);
};

// 使用环形缓冲区和FFT的改进版心率计算器
class HeartRateCalculator {
private:
    // 滤波器和缓冲区参数
    const int SAMPLE_RATE = 15; // fps
    const int BUFFER_SIZE = 256; // 必须是2的幂，方便FFT，约17秒数据
    
    // 环形缓冲区实现零内存分配滑动窗口
    std::vector<double> signalBuffer;
    int headIndex;
    int dataCount;
    
public:
    HeartRateCalculator();
    
    // 主处理函数：添加新样本，并返回计算的心率（如果数据足够）
    double processPPGSignal(double rawSample, double qualityScore = 1.0);

private:
    void updateBuffer(double sample);
    
    // 信号预处理（运动补偿与带通滤波）
    std::vector<double> preprocessSignal();
    
    // OpenMP加速的FFT频谱分析与心率提取
    double calculateHeartRateFFT(const std::vector<double>& processedSignal);
};

