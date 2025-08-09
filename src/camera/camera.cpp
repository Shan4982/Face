#include "camera.hpp"
//bool is_same(const Mat &frame1, const Mat &frame2);

bool  Camera::getCameraFrame(Mat& frame) {
    cap >> frame;
    return true;
}

// bool Camera::sync(Mat &frame)
// {
//     Mat now;
//     cap >> now;
//     if (is_same(frame, now)) {
//         return false;
//     } else {
//         frame = now;
//         return true;
// }
// }

// bool Camera::is_same(const Mat &frame1, const Mat &frame2)
// {
//     if (frame1.size() != frame2.size()) {
//         return false;
//     }
//     for (int i = 0; i < frame1.rows; i++) {
//             if (frame1.at<Vec3b>(i, 0) != frame2.at<Vec3b>(i, 0)) {
//                 return false;
//             }
//     }
//     return true;
// }