#include "face.hpp"

Mat faceDetect::Get_ROI_face(Mat& frame) {
    if (frame.empty()) 
    {
        cerr << "错误：无法读取视频帧！" << endl;
        return frame;
    }
    CascadeClassifier faceCascade;
    if (!faceCascade.load("E:\\face\\data\\haarcascade_frontalface_default.xml")) {
        cerr << "错误：无法加载人脸检测模型！" << endl;
    }

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
        Mat ROI ;
        if(faces.size()!=0)
        resize(frame(faces[0]),ROI,Size(300,300));
        return ROI;
}
