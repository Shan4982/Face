#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Camera {
private:
    VideoCapture cap;
public:
    bool getCameraFrame(Mat& frame);
    Camera(VideoCapture cap): cap(cap) {};
    ~Camera() {
        cap.release();
    }
    bool sync(const Mat &frame);
    bool is_same(const Mat &frame1, const Mat &frame2);
};
