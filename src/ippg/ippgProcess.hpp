#pragma once
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
#include <cmath>
#include <algorithm>
#include <deque>

class ippgProcess {
    public:
        // bool get_frame(Mat &frame);
        // void mean_process();
        // void BandpassFilter(std::vector<double>& signal);
        // double CalculateHeartRate(double fps);
        // bool is_full_frames();
        double get_RawSimple(const Mat& frame);
    private:
        // vector<double> ippgSignal;
        // vector<Mat> frames;

};


class HeartRateCalculator {
private:
    // 滤波器参数
    const double LOW_CUTOFF = 0.5;   // Hz (30 BPM)
    const double HIGH_CUTOFF = 4.0;  // Hz (240 BPM)
    const int SAMPLE_RATE = 30;     // Hz
    
    // 历史数据存储
    std::deque<double> signalBuffer;
    std::deque<double> peakIntervals;
    const int BUFFER_SIZE = 5 * SAMPLE_RATE; // 5秒缓冲区
    
public:
    HeartRateCalculator() {
        signalBuffer.resize(BUFFER_SIZE, 0.0);
    }
    
    // 主处理函数
    double processPPGSignal(double rawSample) {
        // 1. 更新信号缓冲区
        updateBuffer(rawSample);
        
        // 2. 信号预处理
        std::vector<double> processed = preprocessSignal();
        
        // 3. 峰值检测
        std::vector<int> peakIndices = detectPeaks(processed);
        
        // 4. 计算心率
        return calculateHeartRate(peakIndices);
    }

private:
    // 更新环形缓冲区
    void updateBuffer(double sample) {
        signalBuffer.pop_front();
        signalBuffer.push_back(sample);
    }
    
    // 信号预处理
    std::vector<double> preprocessSignal() {
        std::vector<double> processed(BUFFER_SIZE);
        
        // a. 去除直流分量 (移动平均法)
        double movingAvg = 0.0;
        for (double val : signalBuffer) {
            movingAvg += val;
        }
        movingAvg /= BUFFER_SIZE;
        
        // b. 带通滤波 (Butterworth二阶)
        const int order = 2;
        double a[order+1] = {1.0, -1.561, 0.641}; // 预计算系数
        double b[order+1] = {0.020, 0.0, -0.020};
        
        double x[order+1] = {0}; // 输入历史
        double y[order+1] = {0}; // 输出历史
        
        for (int i = 0; i < BUFFER_SIZE; i++) {
            // 更新输入历史
            for (int j = order; j > 0; j--) {
                x[j] = x[j-1];
            }
            x[0] = signalBuffer[i] - movingAvg; // 去直流
            
            // 计算滤波输出
            y[0] = 0.0;
            for (int j = 0; j <= order; j++) {
                y[0] += b[j] * x[j];
            }
            for (int j = 1; j <= order; j++) {
                y[0] -= a[j] * y[j];
            }
            
            // 更新输出历史
            for (int j = order; j > 0; j--) {
                y[j] = y[j-1];
            }
            
            processed[i] = y[0];
        }
        
        return processed;
    }
    
    // 峰值检测算法
    std::vector<int> detectPeaks(const std::vector<double>& signal) {
        std::vector<int> peaks;
        const int windowSize = 6; // 200ms窗口
        
        // 自适应阈值计算
        double threshold = 0.0;
        for (double val : signal) {
            if (val > threshold) threshold = val;
        }
        threshold *= 0.7; // 使用峰值的70%作为阈值
        
        // 滑动窗口检测局部最大值
        for (int i = windowSize; i < signal.size() - windowSize; i++) {
            bool isPeak = true;
            
            // 检查左侧
            for (int j = 1; j <= windowSize; j++) {
                if (signal[i] < signal[i-j] || signal[i] < signal[i+j]) {
                    isPeak = false;
                    break;
                }
            }
            
            // 检查阈值条件
            if (isPeak && signal[i] > threshold) {
                // 确保不重复检测同一峰值区域
                if (peaks.empty() || (i - peaks.back()) > windowSize/2) {
                    peaks.push_back(i);
                }
            }
        }
        
        return peaks;
    }
    
    // 心率计算
    double calculateHeartRate(const std::vector<int>& peakIndices) {
        if (peakIndices.size() < 2) {
            return 0.0; // 不足两个峰值，无法计算
        }
        
        // 1. 计算峰间期(样本数)
        std::vector<double> intervals;
        for (int i = 1; i < peakIndices.size(); i++) {
            intervals.push_back(peakIndices[i] - peakIndices[i-1]);
        }
        
        // 2. 中值滤波去除异常值
        std::vector<double> sortedIntervals = intervals;
        std::sort(sortedIntervals.begin(), sortedIntervals.end());
        double median = sortedIntervals[sortedIntervals.size()/2];
        
        // 3. 过滤异常区间 (偏离中值>20%)
        double validSum = 0.0;
        int validCount = 0;
        for (double interval : intervals) {
            if (std::abs(interval - median) < median * 0.2) {
                validSum += interval;
                validCount++;
            }
        }
        
        if (validCount == 0) return 0.0;
        
        // 4. 计算平均峰间期(秒)
        double avgInterval = (validSum / validCount) / SAMPLE_RATE;
        
        // 5. 转换为心率(BPM)
        double heartRate = 60.0 / avgInterval;
        
        // 6. 更新历史缓冲区
        peakIntervals.push_back(heartRate);
        if (peakIntervals.size() > 5) peakIntervals.pop_front();
        
        // 7. 移动平均滤波
        double sum = 0.0;
        for (double hr : peakIntervals) sum += hr;
        return sum / peakIntervals.size();
    }
};
