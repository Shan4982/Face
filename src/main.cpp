#include "camera/camera.hpp"
#include "face/faceDetect.hpp"
#include "database/faceDB.hpp"
#include "ippg/ippgProcess.hpp"
#include "controller/ThreadChannel.hpp"
#include <thread>
#include<iostream>
#include<mutex>
#include<atomic>
#include<opencv2/dnn.hpp>
using namespace std;

ThreadChannel channel_A_B;
atomic<bool> should_exit(false);

void ippg_process_B(ThreadChannel &channel)
{
    double simple;
    HeartRateCalculator heartRateCalculator;
    while(!should_exit.load())
    {
        simple = channel.receive();
        double heartRate = heartRateCalculator.processPPGSignal(simple);
        cout<<"heartRate:"<<heartRate<<endl;
    }
}


int main(){

    // ThreadChannel channel_A_B;
    // thread thread_A(video_face_A,ref(channel_A_B));
    // while (true) {
    //     if (waitKey(1) == 27) { // ³ÖÐø¼ì²âESC
    //         should_exit.store(true);
    //         break;
    //     }
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }
    // thread_A.join();
    thread thread_B(ippg_process_B,ref(channel_A_B));
    Mat frame;
    Mat ROI;
    Camera camera(VideoCapture(0));
    faceDetect faceDetector;
    ippgProcess ippgProcessor;
    namedWindow("frame");
    int i=0;
    do
    {
        auto t1 = std::chrono::high_resolution_clock::now();
            cout<<i++<<":"<<endl;
            camera.getCameraFrame(frame);
            ROI = faceDetector.Get_ROI_face_DNN(frame);
            imshow("frame",frame);
            double raw_simple = ippgProcessor.get_RawSimple(ROI);
           channel_A_B.send(raw_simple); 
           cout<<"simple:"<<raw_simple<<endl;
           if(waitKey(1)==27) 
           {
            should_exit.store(true);
            break;
           }
           auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Frame time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " ms" << std::endl;
    }while(1);
    channel_A_B.close();
    thread_B.join();
    return 0;
}