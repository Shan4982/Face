#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace cv::face;
using namespace std;

// 人脸数据库操作类（来自insert.cpp）
class FaceDB {
private:
    sqlite3* db;
public:
    FaceDB(const string& dbPath) {
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc) {
            cerr << "无法打开数据库: " << sqlite3_errmsg(db) << endl;
            db = nullptr;
        }
    }

    ~FaceDB() {
        if (db) sqlite3_close(db);
    }

    bool insertFace(const string& name, const vector<float>& feature) {
        if (!db) return false;
        const void* blobData = feature.data();
        int blobSize = feature.size() * sizeof(float);

        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO faces (name, feature) VALUES (?, ?);";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_blob(stmt, 2, blobData, blobSize, SQLITE_STATIC);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    vector<pair<string, float>> searchFace(const vector<float>& targetFeature) {
        vector<pair<string, float>> results;
        if (!db) return results;

        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, name, feature FROM faces;";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const void* blobData = sqlite3_column_blob(stmt, 2);
            int blobSize = sqlite3_column_bytes(stmt, 2);
            int featureSize = blobSize / sizeof(float);

            vector<float> storedFeature(featureSize);
            memcpy(storedFeature.data(), blobData, blobSize);

            // 计算余弦相似度
            float similarity = 0, norm1 = 0, norm2 = 0;
            for (size_t i = 0; i < featureSize && i < targetFeature.size(); i++) {
                similarity += targetFeature[i] * storedFeature[i];
                norm1 += targetFeature[i] * targetFeature[i];
                norm2 += storedFeature[i] * storedFeature[i];
            }
            similarity = similarity / (sqrt(norm1) * sqrt(norm2) + 1e-6);
            results.emplace_back(name, similarity);
        }
        sqlite3_finalize(stmt);
        return results;
    }
};

// 人脸特征提取函数
vector<float> extractFaceFeature(const Mat& faceImage) {
    Mat gray;
    cvtColor(faceImage, gray, COLOR_BGR2GRAY);
    resize(gray, gray, Size(92, 112)); // 统一尺寸

    // 使用LBPH特征（实际应用建议使用深度学习模型）
    Ptr<LBPHFaceRecognizer> recognizer = LBPHFaceRecognizer::create();
    vector<Mat> images = {gray};
    vector<int> labels = {0};
    recognizer->train(images, labels);

    // 获取特征向量
    Mat featureMat = recognizer->getHistograms()[0];
    return vector<float>(featureMat.begin<float>(), featureMat.end<float>());
}

int main() {
    // 1. 初始化人脸检测模型
    CascadeClassifier faceCascade;
    if (!faceCascade.load("E:/astu/cv/project/haarcascade_frontalface_default.xml")) {
        cerr << "错误：无法加载人脸检测模型！" << endl;
        return -1;
    }

    // 2. 初始化人脸数据库
    FaceDB faceDB("face_database.db");

    // 3. 初始化摄像头
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "错误：无法访问摄像头！" << endl;
        return -1;
    }

    Mat frame, grayFrame;
    cout << "请按ESC键退出，按空格键保存人脸..." << endl;

    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        // 4. 人脸检测
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
        equalizeHist(grayFrame, grayFrame);

        vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, Size(30, 30));

        // 5. 处理检测到的人脸
        for (const Rect& face : faces) {
            // 提取人脸区域
            Mat faceROI = frame(face);
            vector<float> feature = extractFaceFeature(faceROI);

            // 6. 人脸识别（搜索数据库）
            string name = "未知";
            auto matches = faceDB.searchFace(feature);
            if (!matches.empty()) {
                // 取相似度最高的结果
                sort(matches.begin(), matches.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
                if (matches[0].second > 0.6) { // 相似度阈值
                    name = matches[0].first + "(" + to_string(int(matches[0].second*100)) + "%)";
                }
            }

            // 7. 绘制结果
            rectangle(frame, face.tl(), face.br(), Scalar(0, 255, 0), 2);
            putText(frame, name, Point(face.x, face.y-10), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
        }

        // 8. 交互控制
        imshow("人脸识别", frame);
        int key = waitKey(10);
        if (key == 27) break; // ESC退出
        if (key == 32 && !faces.empty()) { // 空格键保存人脸
            string name;
            cout << "请输入姓名: ";
            cin >> name;
            vector<float> feature = extractFaceFeature(frame(faces[0]));
            if (faceDB.insertFace(name, feature)) {
                cout << "人脸特征保存成功！" << endl;
            }
        }
    }

    return 0;
}
