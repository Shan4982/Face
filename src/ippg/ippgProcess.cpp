#include "ippgProcess.hpp"

// std::vector<double> ippgSignal; // Define datalist as a global variable
// std::vector<Mat> frames; // Define frames as a global variable

// void ippgProcess::mean_process() {
//     Mat frame;
//     vector<Mat> channels;
//     for(int i=0;i<frames.size();i++)
//     {  // Vector to store the three channels of the image
//     split(frames[1],channels);
//     Mat green;  // Result image after processing
//     green = channels[1];  // Get the green channel of the image
//     double mean_value;
//     mean_value = mean(green)[0];  // Calculate the mean value of the green channel
//     ippgSignal.push_back(mean_value);  // Add the mean value to the data list
//     }
//         const int window_size = 15; // 滑动窗口大小
//         std::vector<double> filtered(ippgSignal.size());
        
//         // 滑动窗口均值滤波
//         for(int i = 0; i < ippgSignal.size(); ++i) {
//             int start = std::max(0, (int)i - window_size/2);
//             int end = std::min((int)ippgSignal.size()-1,i + window_size/2);
            
//             double sum = 0.0;
//             for(int j = start; j <= end; ++j) {
//                 sum += ippgSignal[j];
//             }
//             filtered[i] = sum / (end - start + 1);
//         }
//         ippgSignal = filtered;

//         double max_value = *std::max_element(ippgSignal.begin(), ippgSignal.end()); // 找到最大值
//         double min_value = *std::min_element(ippgSignal.begin(), ippgSignal.end()); // 找到最小值
//         // 归一化处理
//         for(int i=0;i<ippgSignal.size();i++)
//         {
//             ippgSignal[i] = ((ippgSignal[i]-min_value)/(max_value-min_value));
//         }
// }



// bool ippgProcess::get_frame(Mat &frame) {

//     frames.push_back(frame);  // Add the frame to the frames vector
//     return true;
// }


// void ippgProcess::BandpassFilter(std::vector<double>& signal) {
//     // 1. 傅里叶变换
//     cv::Mat input(signal.size(), 1, CV_64F, signal.data());
//     cv::Mat planes[] = {input.clone(), cv::Mat::zeros(input.size(), CV_64F)};
//     cv::Mat complex;
//     cv::merge(planes, 2, complex);
//     cv::dft(complex, complex);
    
//     // 2. 构造带通滤波器 (0.8-4Hz)
//     cv::Mat mask = cv::Mat::zeros(complex.size(), CV_64F);
//     int lowIdx = 0.8 * signal.size() / 30;  // fps为视频帧率
//     int highIdx = 4 * signal.size() / 30;
//     cv::rectangle(mask, cv::Point(lowIdx,0), cv::Point(highIdx, mask.rows), 1, -1);
    
//     // 3. 频域滤波
//     cv::Mat filtered;
//     cv::multiply(complex, mask, filtered);
    
//     // 4. 逆变换还原信号
//     cv::idft(filtered, filtered, cv::DFT_REAL_OUTPUT);
//     cv::normalize(filtered, filtered, 0, 1, cv::NORM_MINMAX);
//     filtered.copyTo(input);
// }


// double  ippgProcess::CalculateHeartRate(double fps) {
//     // 1. 计算FFT
//     cv::Mat input(ippgSignal.size(), 1, CV_64F, (void*)ippgSignal.data());
//     cv::Mat spectrum;
//     cv::dft(input, spectrum, cv::DFT_COMPLEX_OUTPUT);

//     // 2. 找主频峰值 (跳过直流分量)
//     cv::Point maxLoc;
//     cv::Mat powerSpectrum;
//     cv::magnitude(spectrum.col(0), spectrum.col(1), powerSpectrum);
//     minMaxLoc(powerSpectrum.rowRange(1, powerSpectrum.rows/2), NULL, NULL, NULL, &maxLoc);

//     // 3. 转换为心率 (BPM)
//     double peakFreq = maxLoc.y * fps / ippgSignal.size();
//     return peakFreq * 60.0;  // Hz → BPM
// }

// bool ippgProcess::is_full_frames() {
//     return frames.size() >= 150;
// }

double ippgProcess::get_RawSimple(const Mat& frame)
{
    if(frame.empty())
    {
        cout<<"frame is empty"<<endl;
    return 0.0; // 检查图像是否为空
    }
    vector<Mat> channels;
    split(frame,channels);
    Mat green;
    green = channels[1];
    double mean_value;
    mean_value = mean(green)[0];
    return mean_value;
}//计算这个图像的绿色通道的中值