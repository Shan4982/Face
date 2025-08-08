#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class faceDetect {
private:

    public:
    Mat Get_ROI_face(Mat& src);
};