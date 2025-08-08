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
    // �˲�������
    const double LOW_CUTOFF = 0.5;   // Hz (30 BPM)
    const double HIGH_CUTOFF = 4.0;  // Hz (240 BPM)
    const int SAMPLE_RATE = 30;     // Hz
    
    // ��ʷ���ݴ洢
    std::deque<double> signalBuffer;
    std::deque<double> peakIntervals;
    const int BUFFER_SIZE = 5 * SAMPLE_RATE; // 5�뻺����
    
public:
    HeartRateCalculator() {
        signalBuffer.resize(BUFFER_SIZE, 0.0);
    }
    
    // ��������
    double processPPGSignal(double rawSample) {
        // 1. �����źŻ�����
        updateBuffer(rawSample);
        
        // 2. �ź�Ԥ����
        std::vector<double> processed = preprocessSignal();
        
        // 3. ��ֵ���
        std::vector<int> peakIndices = detectPeaks(processed);
        
        // 4. ��������
        return calculateHeartRate(peakIndices);
    }

private:
    // ���»��λ�����
    void updateBuffer(double sample) {
        signalBuffer.pop_front();
        signalBuffer.push_back(sample);
    }
    
    // �ź�Ԥ����
    std::vector<double> preprocessSignal() {
        std::vector<double> processed(BUFFER_SIZE);
        
        // a. ȥ��ֱ������ (�ƶ�ƽ����)
        double movingAvg = 0.0;
        for (double val : signalBuffer) {
            movingAvg += val;
        }
        movingAvg /= BUFFER_SIZE;
        
        // b. ��ͨ�˲� (Butterworth����)
        const int order = 2;
        double a[order+1] = {1.0, -1.561, 0.641}; // Ԥ����ϵ��
        double b[order+1] = {0.020, 0.0, -0.020};
        
        double x[order+1] = {0}; // ������ʷ
        double y[order+1] = {0}; // �����ʷ
        
        for (int i = 0; i < BUFFER_SIZE; i++) {
            // ����������ʷ
            for (int j = order; j > 0; j--) {
                x[j] = x[j-1];
            }
            x[0] = signalBuffer[i] - movingAvg; // ȥֱ��
            
            // �����˲����
            y[0] = 0.0;
            for (int j = 0; j <= order; j++) {
                y[0] += b[j] * x[j];
            }
            for (int j = 1; j <= order; j++) {
                y[0] -= a[j] * y[j];
            }
            
            // ���������ʷ
            for (int j = order; j > 0; j--) {
                y[j] = y[j-1];
            }
            
            processed[i] = y[0];
        }
        
        return processed;
    }
    
    // ��ֵ����㷨
    std::vector<int> detectPeaks(const std::vector<double>& signal) {
        std::vector<int> peaks;
        const int windowSize = 6; // 200ms����
        
        // ����Ӧ��ֵ����
        double threshold = 0.0;
        for (double val : signal) {
            if (val > threshold) threshold = val;
        }
        threshold *= 0.7; // ʹ�÷�ֵ��70%��Ϊ��ֵ
        
        // �������ڼ��ֲ����ֵ
        for (int i = windowSize; i < signal.size() - windowSize; i++) {
            bool isPeak = true;
            
            // ������
            for (int j = 1; j <= windowSize; j++) {
                if (signal[i] < signal[i-j] || signal[i] < signal[i+j]) {
                    isPeak = false;
                    break;
                }
            }
            
            // �����ֵ����
            if (isPeak && signal[i] > threshold) {
                // ȷ�����ظ����ͬһ��ֵ����
                if (peaks.empty() || (i - peaks.back()) > windowSize/2) {
                    peaks.push_back(i);
                }
            }
        }
        
        return peaks;
    }
    
    // ���ʼ���
    double calculateHeartRate(const std::vector<int>& peakIndices) {
        if (peakIndices.size() < 2) {
            return 0.0; // ����������ֵ���޷�����
        }
        
        // 1. ��������(������)
        std::vector<double> intervals;
        for (int i = 1; i < peakIndices.size(); i++) {
            intervals.push_back(peakIndices[i] - peakIndices[i-1]);
        }
        
        // 2. ��ֵ�˲�ȥ���쳣ֵ
        std::vector<double> sortedIntervals = intervals;
        std::sort(sortedIntervals.begin(), sortedIntervals.end());
        double median = sortedIntervals[sortedIntervals.size()/2];
        
        // 3. �����쳣���� (ƫ����ֵ>20%)
        double validSum = 0.0;
        int validCount = 0;
        for (double interval : intervals) {
            if (std::abs(interval - median) < median * 0.2) {
                validSum += interval;
                validCount++;
            }
        }
        
        if (validCount == 0) return 0.0;
        
        // 4. ����ƽ�������(��)
        double avgInterval = (validSum / validCount) / SAMPLE_RATE;
        
        // 5. ת��Ϊ����(BPM)
        double heartRate = 60.0 / avgInterval;
        
        // 6. ������ʷ������
        peakIntervals.push_back(heartRate);
        if (peakIntervals.size() > 5) peakIntervals.pop_front();
        
        // 7. �ƶ�ƽ���˲�
        double sum = 0.0;
        for (double hr : peakIntervals) sum += hr;
        return sum / peakIntervals.size();
    }
};
