#include "faceDetect.hpp"

Mat faceDetect::Get_ROI_face(Mat& frame) {
    Mat ROI;
    if (frame.empty()) 
    {
        cerr << "错误：无法读取视频帧！" << endl;
        return ROI=Mat();
    }
    //resize(frame, frame, Size(frame.cols/2, frame.rows/2));

    Mat grayFrame;

        // 转换为灰度图（人脸检测需要）
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
        equalizeHist(grayFrame, grayFrame);  // 增强对比度

        // 检测人脸
        vector<Rect> faces;
        faceCascade.detectMultiScale(
            grayFrame, faces,
            1.1,    // 缩放因子
            3,      // 最小邻居数
            0,      // 标志位（保留）
            Size(30, 30)  // 最小人脸尺寸
        );

        // 绘制人脸矩形框
        for (const Rect& face : faces) {
            rectangle(
                frame, 
                face.tl(), face.br(),  // 左上角/右下角坐标
                Scalar(0, 255, 0),    // 绿色边框
                2                     // 线宽
            );
        }
        if(faces.size()!=0)
        // resize(frame(faces[0]),ROI,Size(200,200));
        ROI=frame(faces[0]);
        else return ROI=Mat();
        ROI=ROI(Rect(ROI.cols/4,ROI.rows/4,ROI.cols/2,ROI.rows/2));
        return ROI;
};


Mat faceDetect::Get_ROI_face_DNN(Mat& src) {
    Mat ROI;
        if (src.empty()) {
            cerr << "错误：无法读取视频帧！" << endl;
            return ROI=Mat();
        }
        // DNN前处理
        Mat blob = dnn::blobFromImage(src, 1.0, Size(300, 300), Scalar(104.0, 177.0, 123.0), false, false);
        net.setInput(blob);
        Mat detections = net.forward();

        // 解析检测结果
        Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
        vector<Rect> faces;
        for (int i = 0; i < detectionMat.rows; i++) {
            float confidence = detectionMat.at<float>(i, 2);
            if (confidence > confThreshold) {
                int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * src.cols);
                int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * src.rows);
                int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * src.cols);
                int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * src.rows);
                Rect faceBox(Point(x1, y1), Point(x2, y2));
                faces.push_back(faceBox);
                rectangle(src, faceBox, Scalar(0, 255, 0), 2);
            }
        }
        // 取第一个人脸作为ROI
        if (!faces.empty()) {
            ROI = src(faces[0]);
            return ROI;
        } else {
            ROI = Mat();
            return ROI;
        }
    }