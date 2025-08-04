#include "ippgProcess.hpp"

void ippgProcess::process(Mat & frame) {
    
}

std::vector<double> ippgSignal; // Define datalist as a global variable
std::vector<Mat> frames; // Define frames as a global variable

void mean_process() {
    Mat frame;
    vector<Mat> channels;
    for(int i=0;i<frames.size();i++)
    {  // Vector to store the three channels of the image
    split(frame,channels);
    Mat green;  // Result image after processing
    green = channels[1];  // Get the green channel of the image
    double mean_value;
    mean_value = mean(green)[0];  // Calculate the mean value of the green channel
    ippgSignal.push_back(mean_value);  // Add the mean value to the data list
    }
        const int window_size = 15; // 滑动窗口大小
        std::vector<double> filtered(ippgSignal.size());
        
        // 滑动窗口均值滤波
        for(size_t i = 0; i < ippgSignal.size(); ++i) {
            int start = std::max(0, (int)i - window_size/2);
            int end = std::min((int)ippgSignal.size()-1, (int)i + window_size/2);
            
            double sum = 0.0;
            for(int j = start; j <= end; ++j) {
                sum += ippgSignal[j];
            }
            filtered[i] = sum / (end - start + 1);
        }
        ippgSignal = filtered;

        double max_value = *std::max_element(ippgSignal.begin(), ippgSignal.end()); // 找到最大值
        double min_value = *std::min_element(ippgSignal.begin(), ippgSignal.end()); // 找到最小值
        // 归一化处理
        for(int i=0;i<ippgSignal.size();i++)
        {
            ippgSignal[i] = ((ippgSignal[i]-min_value)/(max_value-min_value));
        }
}



void ippgProcess::get_frame(Mat &frame) {
    frames.push_back(frame);  // Add the frame to the frames vector
    if(frames.size()>=MAX_FRAME) 
    {
        cout<<"frames full"<<endl;
    } // When the number of frames reaches 10, perform the mean processing
}