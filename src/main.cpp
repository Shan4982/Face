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
    // ����ͷ��ʼ��
    Camera camera(VideoCapture(0));

    // ��������������ȡ
    HeartRateCalculator heartRateCalculator;
    faceDetect faceDetector;
    ippgProcess ippgProcessor;
    // ���ڳ�ʼ��
    namedWindow("frame");
    
    for(int i = 0; i<1000;i++)
    {
        // ��ȡ����ͷ֡
        if (!camera.getCameraFrame(frame)) {
            cerr << "Error: Failed to capture frame from camera." << endl;
            continue;
        }

        // �����������ȡ����
        Mat roiFace = faceDetector.Get_ROI_face(frame);

        // ��ʾԭʼ֡
        imshow("frame", frame);

        double simple = ippgProcessor.get_RawSimple(roiFace);

        // ��������
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