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
        void get_frame(Mat &frame);
        void mean_process();
    private:
        vector<double> ippgSingal;
        vector<Mat> frames;
};