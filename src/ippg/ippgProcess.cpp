#include "ippgProcess.hpp"

// ==================== SignalSimulator ====================
double SignalSimulator::generateSample() {
    t += dt;
    // 基础心跳信号
    double ppg = std::sin(2.0 * CV_PI * (true_hr / 60.0) * t);
    // 基线漂移 (低频噪声)
    double baseline = 0.5 * std::sin(2.0 * CV_PI * 0.1 * t);
    // 高斯白噪声
    double noise = 0.1 * ((rand() % 100) / 50.0 - 1.0);
    // 模拟运动伪影 (偶尔的脉冲)
    double motion = (rand() % 1000 < 5) ? 2.0 : 0.0;
    
    return ppg + baseline + noise + motion;
}

// ==================== ippgProcess ====================
double ippgProcess::get_RawSimple(const Mat& frame) {
    if (frame.empty()) {
        // cout << "frame is empty" << endl;
        return 0.0;
    }
    vector<Mat> channels;
    split(frame, channels);
    Mat green = channels[1]; // 获取绿色通道
    return mean(green)[0];   // 返回绿通道均值
}

double ippgProcess::evaluateSignalQuality(const Mat& roi) {
    if (roi.empty()) return 0.0;
    vector<Mat> channels;
    split(roi, channels);
    Mat green = channels[1];
    Scalar mean_val, stddev_val;
    meanStdDev(green, mean_val, stddev_val);
    // 方差越小，ROI颜色一致性越好，信号质量越高
    double variance = stddev_val.val[0] * stddev_val.val[0];
    // 归一化评分 (启发式)
    double score = 1.0 / (1.0 + variance * 0.01);
    return score;
}

// ==================== HeartRateCalculator ====================
HeartRateCalculator::HeartRateCalculator() : headIndex(0), dataCount(0) {
    signalBuffer.resize(BUFFER_SIZE, 0.0);
}

void HeartRateCalculator::updateBuffer(double sample) {
    signalBuffer[headIndex] = sample;
    headIndex = (headIndex + 1) % BUFFER_SIZE;
    if (dataCount < BUFFER_SIZE) dataCount++;
}

double HeartRateCalculator::processPPGSignal(double rawSample, double qualityScore) {
    // 质量过低时可以拒绝更新，这里作为演示直接记录
    if (qualityScore < 0.1) return 0.0;
    
    updateBuffer(rawSample);
    
    if (dataCount < BUFFER_SIZE) return 0.0;
    
    std::vector<double> processed = preprocessSignal();
    return calculateHeartRateFFT(processed);
}

std::vector<double> HeartRateCalculator::preprocessSignal() {
    std::vector<double> linearBuffer(BUFFER_SIZE);
    
    // 展开环形缓冲区
    for (int i = 0; i < BUFFER_SIZE; i++) {
        linearBuffer[i] = signalBuffer[(headIndex + i) % BUFFER_SIZE];
    }
    
    double movingAvg = 0.0;
    // 使用 OpenMP 加速均值计算
    #pragma omp parallel for reduction(+:movingAvg)
    for (int i = 0; i < BUFFER_SIZE; i++) {
        movingAvg += linearBuffer[i];
    }
    movingAvg /= BUFFER_SIZE;
    
    std::vector<double> processed(BUFFER_SIZE);
    // 运动补偿与去直流
    #pragma omp parallel for
    for (int i = 0; i < BUFFER_SIZE; i++) {
        // 简单运动补偿：裁剪极值
        double val = linearBuffer[i] - movingAvg;
        if (val > 10.0) val = 10.0;
        if (val < -10.0) val = -10.0;
        processed[i] = val;
    }
    
    return processed;
}

double HeartRateCalculator::calculateHeartRateFFT(const std::vector<double>& processedSignal) {
    // 准备 OpenCV dft 输入
    Mat input(BUFFER_SIZE, 1, CV_64F);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        // 加汉明窗(Hamming window)减少频谱泄漏
        double window = 0.54 - 0.46 * std::cos(2.0 * CV_PI * i / (BUFFER_SIZE - 1));
        input.at<double>(i, 0) = processedSignal[i] * window;
    }
    
    Mat spectrum;
    dft(input, spectrum, DFT_COMPLEX_OUTPUT);
    
    // 计算幅度谱
    std::vector<double> magnitude(BUFFER_SIZE / 2);
    // 使用 OpenMP 并行计算频谱幅度
    #pragma omp parallel for
    for (int i = 0; i < BUFFER_SIZE / 2; i++) {
        double re = spectrum.at<Vec2d>(i, 0)[0];
        double im = spectrum.at<Vec2d>(i, 0)[1];
        magnitude[i] = std::sqrt(re * re + im * im);
    }
    
    // 寻找心率带范围内的峰值 (0.8Hz - 3.0Hz) -> (48 BPM - 180 BPM)
    double minFreq = 0.8;
    double maxFreq = 3.0;
    int minIdx = std::max(1, (int)(minFreq * BUFFER_SIZE / SAMPLE_RATE));
    int maxIdx = std::min(BUFFER_SIZE / 2 - 1, (int)(maxFreq * BUFFER_SIZE / SAMPLE_RATE));
    
    double maxMag = 0;
    int peakIdx = minIdx;
    
    for (int i = minIdx; i <= maxIdx; i++) {
        if (magnitude[i] > maxMag) {
            maxMag = magnitude[i];
            peakIdx = i;
        }
    }
    
    double peakFreq = (double)peakIdx * SAMPLE_RATE / BUFFER_SIZE;
    return peakFreq * 60.0; // 转换为BPM
}
