#include "camera/camera.hpp"
#include "face/face.hpp"
#include "database/faceDB.hpp"
#include "ippg/ippgProcess.hpp"
#include "controller/ThreadChannel.hpp"
#include <thread>
#include<iostream>
#include<future>
#include<mutex>
using namespace std;



void video_face(ThreadChannel &channel)
{
    Mat frame;
    Camera camera(VideoCapture(0));
    while(1)
    {
        if(camera.sync(frame))
        {
            promise <Mat> p;
            if(camera.getCameraFrame(frame)){
                p.set_value(frame);
            }
        }
    }
}

void ippg_process(ThreadChannel &channel)
{

}
int main(){
Mat frame;
    // 摄像头初始化
    Camera camera(VideoCapture(0));

    // 人脸检测和特征提取
    HeartRateCalculator heartRateCalculator;
    faceDetect faceDetector;
    ippgProcess ippgProcessor;
    // 窗口初始化
    namedWindow("frame");
    
    for(int i = 0; i<1000;i++)
    {
        // 读取摄像头帧
        if (!camera.getCameraFrame(frame)) {
            cerr << "Error: Failed to capture frame from camera." << endl;
            continue;
        }

        // 检测人脸并提取特征
        Mat roiFace = faceDetector.Get_ROI_face(frame);

        // 显示原始帧
        imshow("frame", frame);

        double simple = ippgProcessor.get_RawSimple(roiFace);

        // 计算心率
        double heartRate = heartRateCalculator.processPPGSignal(simple);
        if(i/30==0){
        cout<<i<<endl;
        cout << "Heart Rate: " << fixed << setprecision(1) << heartRate << " bpm" << endl;
        }

        if(waitKey(1)=='q'){
            break;
        }
    }
}