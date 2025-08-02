#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Camera {
private:
    VideoCapture cap;
    public:
    Mat& getCameraFrame(Mat& frame);
    Camera(VideoCapture cap): cap(cap) {};
};