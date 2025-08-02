#include "camera.hpp"

Mat& Camera::getCameraFrame(Mat& frame) {
    cap >> frame;
    return frame;
}