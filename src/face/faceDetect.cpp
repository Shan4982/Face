#include "faceDetect.hpp"

// 模糊检测算法：计算图像拉普拉斯算子的方差
bool faceDetect::isBlurry(const Mat& src) {
    if (src.empty()) return true;
    Mat gray, lap;
    if (src.channels() == 3) {
        cvtColor(src, gray, COLOR_BGR2GRAY);
    } else {
        gray = src;
    }
    // 使用拉普拉斯算子
    Laplacian(gray, lap, CV_64F);
    Scalar mu, sigma;
    meanStdDev(lap, mu, sigma);
    double variance = sigma.val[0] * sigma.val[0];
    // 如果方差低于阈值，说明图像边缘信息少，被认为是模糊的
    return variance < blurThreshold;
}

Mat faceDetect::Get_ROI_face(Mat& frame) {
    Mat ROI;
    if (frame.empty()) {
        cerr << "错误：无法读取视频帧！" << endl;
        return ROI;
    }

    // 性能优化：缩小检测尺寸 (检测时使用较小分辨率，提取 ROI 时映射回原图)
    Mat smallFrame;
    double scale = 0.5;
    resize(frame, smallFrame, Size(), scale, scale);

    // 引入OpenCL后端提升运算速度
    UMat uGray;
    cvtColor(smallFrame, uGray, COLOR_BGR2GRAY);
    equalizeHist(uGray, uGray);

    // 检测人脸
    vector<Rect> faces;
    faceCascade.detectMultiScale(
        uGray, faces,
        1.1,    // 缩放因子
        5,      // 最小邻居数
        0,      // 标志位
        Size(30, 30)  // 在缩小后的图中检测 30x30 的脸
    );

    if (faces.empty()) {
        return ROI;
    }

    // 将检测到的矩形映射回原图尺寸
    Rect faceRect(faces[0].x / scale, faces[0].y / scale, faces[0].width / scale, faces[0].height / scale);
    
    // 绘制人脸框
    rectangle(frame, faceRect, Scalar(0, 255, 0), 2);
    
    // 截取人脸区域作为ROI
    Mat faceROI = frame(faceRect);

    // 模糊检测，如果图像模糊则返回空
    if (isBlurry(faceROI)) {
        // cout << "检测到模糊帧，丢弃" << endl;
        return ROI;
    }

    // 进一步截取额头或脸颊部分作为iPPG的标准化ROI
    // 例如：取人脸中心偏下的矩形作为面颊ROI
    int roi_w = faceROI.cols / 2;
    int roi_h = faceROI.rows / 2;
    int roi_x = faceROI.cols / 4;
    int roi_y = faceROI.rows / 4;
    ROI = faceROI(Rect(roi_x, roi_y, roi_w, roi_h)).clone();

    return ROI;
}

