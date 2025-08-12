#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class faceDetect {
private:
    CascadeClassifier faceCascade;
    dnn::Net net;
    float confThreshold = 0.5; 
public:
    faceDetect(){
        if (!faceCascade.load("E:\\face\\data\\haarcascade_frontalface_default.xml")) {
            cerr << "�����޷������������ģ�ͣ�" << endl;
        }
        net = dnn::readNetFromCaffe(
            "E:/face/data/deploy.prototxt",
            "E:/face/data/res10_300x300_ssd_iter_140000.caffemodel"
        );
        if (net.empty()) {
            cerr << "����:�޷�����DNN�������ģ��:" << endl;
        }
    }
    Mat Get_ROI_face(Mat& src);
    Mat Get_ROI_face_DNN(Mat& src);
};