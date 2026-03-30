#include "camera/camera.hpp"
#include "face/faceDetect.hpp"
#include "ippg/ippgProcess.hpp"
#include "controller/ThreadChannel.hpp"
#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>
#include <opencv2/dnn.hpp>
#include <opencv2/core/ocl.hpp> 
#include <memory>

using namespace std;
using namespace cv;

// 定义跨模块传输的数据结构 (零拷贝使用)
struct IPPGData {
    double raw_signal;
    double quality_score;
    // 可以扩展传递Mat： shared_ptr<Mat> faceROI;
};

ThreadChannel<IPPGData> channel_A_B;
atomic<bool> should_exit(false);
atomic<double> current_heart_rate(0.0); // 用于在主线程UI显示

void ippg_process_B(ThreadChannel<IPPGData> &channel) {
    HeartRateCalculator heartRateCalculator;
    while (!should_exit.load()) {
        auto data = channel.receive(1000); // 1秒超时，死锁检测
        if (data == nullptr) {
            if (should_exit.load()) break;
            continue;
        }
        
        // 处理信号
        double heartRate = heartRateCalculator.processPPGSignal(data->raw_signal, data->quality_score);
        if (heartRate > 0) {
            current_heart_rate.store(heartRate);
            cout << "[iPPG 模块] 计算得到心率: " << heartRate << " BPM" << endl;
        }
    }
}

int main() {
    // 启动 OpenCL 加速
    if (cv::ocl::haveOpenCL()) {
        cv::ocl::setUseOpenCL(true);
        cout << "OpenCL 加速已启用: " << (cv::ocl::useOpenCL() ? "Yes" : "No") << endl;
    }

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "错误：无法打开摄像头！" << endl;
        return -1;
    }
    // 尝试设置较低分辨率以提升实时性
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    // 启动处理线程
    thread thread_B(ippg_process_B, ref(channel_A_B));
    
    Mat frame;
    Mat current_ROI; // 缓存当前有效的 ROI
    Camera camera(cap);
    faceDetect faceDetector;
    ippgProcess ippgProcessor;
    SignalSimulator simulator;
    
    namedWindow("Face & iPPG Monitoring", WINDOW_NORMAL);
    
    int frame_count = 0;
    while (!should_exit.load()) {
        if (!cap.read(frame)) {
            cerr << "无法获取摄像头画面！" << endl;
            break;
        }

        // 1. 李思彤模块：人脸检测与ROI提取 (性能优化：每 5 帧重新定位一次人脸)
        if (frame_count % 5 == 0) {
            Mat new_ROI = faceDetector.Get_ROI_face(frame);
            if (!new_ROI.empty()) {
                current_ROI = new_ROI;
            } else {
                current_ROI = Mat(); // 没搜到人脸，重置 ROI
            }
        }
        
        double raw_simple = 0.0;
        double quality = 0.0;

        // 2. 吴浩模块：信号提取与质量评估
        if (!current_ROI.empty()) {
            raw_simple = ippgProcessor.get_RawSimple(current_ROI);
            quality = ippgProcessor.evaluateSignalQuality(current_ROI);
            putText(frame, "Face Tracking", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 2);
        } else {
            raw_simple = simulator.generateSample();
            quality = 1.0; 
            putText(frame, "No Face - Simulator", Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
        }

        // 3. 张少寒模块：零拷贝发送数据 (保持 15-30fps 的采样率对 FFT 很重要)
        auto data = std::make_shared<IPPGData>();
        data->raw_signal = raw_simple;
        data->quality_score = quality;
        channel_A_B.send(data);

        // UI 更新
        double hr = current_heart_rate.load();
        string hr_text = hr > 0 ? cv::format("Heart Rate: %.1f BPM", hr) : "Heart Rate: Calculating...";
        putText(frame, hr_text, Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);
        putText(frame, cv::format("FPS: %d", (int)(1000.0 / 33.0)), Point(10, 120), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);

        imshow("Face & iPPG Monitoring", frame);
        
        frame_count++;
        int key = waitKey(1); // 缩短等待时间，提升响应
        if (key == 27) { 
            should_exit.store(true);
            break;
        }
    }

    // 优雅退出
    channel_A_B.close();
    if (thread_B.joinable()) {
        thread_B.join();
    }
    destroyAllWindows();
    return 0;
}
