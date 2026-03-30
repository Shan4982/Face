#pragma once
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class faceDetect {
private:
    CascadeClassifier faceCascade; // 用于LBP级联
    float blurThreshold = 100.0; // 模糊检测阈值

public:
    faceDetect() {
        // 尝试加载LBP特征级联分类器
        if (!faceCascade.load("E:\\face\\data\\lbpcascade_frontalface_improved.xml")) {
            // 回退到默认Haar或者其他路径
            if (!faceCascade.load("E:\\face\\data\\haarcascade_frontalface_default.xml")) {
                cerr << "错误：无法加载人脸检测模型！" << endl;
            } else {
                cout << "警告: LBP模型加载失败，已回退到Haar模型" << endl;
            }
        } else {
            cout << "成功加载 LBP 级联分类器" << endl;
        }
    }

    // 模糊检测 (Variance of Laplacian)
    bool isBlurry(const Mat& src);

    // 标准化ROI提取接口，使用LBP级联检测
    Mat Get_ROI_face(Mat& src);
};

