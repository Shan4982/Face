#pragma once
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
#define MAX_FRAME 100

class ippgProcess {
    public:
        void process(Mat & frame);
        bool get_frame(Mat &frame);
        void mean_process();
        void BandpassFilter(std::vector<double>& signal);
        double CalculateHeartRate(const std::vector<double>& signal, double fps);
    private:
        vector<double> ippgSingal;
        vector<Mat> frames;
};