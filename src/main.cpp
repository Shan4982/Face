#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace cv::face;
using namespace std;

// �������ݿ�����ࣨ����insert.cpp��
class FaceDB {
private:
    sqlite3* db;
public:
    FaceDB(const string& dbPath) {
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc) {
            cerr << "�޷������ݿ�: " << sqlite3_errmsg(db) << endl;
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

            // �����������ƶ�
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

// ����������ȡ����
vector<float> extractFaceFeature(const Mat& faceImage) {
    Mat gray;
    cvtColor(faceImage, gray, COLOR_BGR2GRAY);
    resize(gray, gray, Size(92, 112)); // ͳһ�ߴ�

    // ʹ��LBPH������ʵ��Ӧ�ý���ʹ�����ѧϰģ�ͣ�
    Ptr<LBPHFaceRecognizer> recognizer = LBPHFaceRecognizer::create();
    vector<Mat> images = {gray};
    vector<int> labels = {0};
    recognizer->train(images, labels);

    // ��ȡ��������
    Mat featureMat = recognizer->getHistograms()[0];
    return vector<float>(featureMat.begin<float>(), featureMat.end<float>());
}

int main() {
    // 1. ��ʼ���������ģ��
    CascadeClassifier faceCascade;
    if (!faceCascade.load("E:/astu/cv/project/haarcascade_frontalface_default.xml")) {
        cerr << "�����޷������������ģ�ͣ�" << endl;
        return -1;
    }

    // 2. ��ʼ���������ݿ�
    FaceDB faceDB("face_database.db");

    // 3. ��ʼ������ͷ
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "�����޷���������ͷ��" << endl;
        return -1;
    }

    Mat frame, grayFrame;
    cout << "�밴ESC���˳������ո����������..." << endl;

    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        // 4. �������
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
        equalizeHist(grayFrame, grayFrame);

        vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, Size(30, 30));

        // 5. �����⵽������
        for (const Rect& face : faces) {
            // ��ȡ��������
            Mat faceROI = frame(face);
            vector<float> feature = extractFaceFeature(faceROI);

            // 6. ����ʶ���������ݿ⣩
            string name = "δ֪";
            auto matches = faceDB.searchFace(feature);
            if (!matches.empty()) {
                // ȡ���ƶ���ߵĽ��
                sort(matches.begin(), matches.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
                if (matches[0].second > 0.6) { // ���ƶ���ֵ
                    name = matches[0].first + "(" + to_string(int(matches[0].second*100)) + "%)";
                }
            }

            // 7. ���ƽ��
            rectangle(frame, face.tl(), face.br(), Scalar(0, 255, 0), 2);
            putText(frame, name, Point(face.x, face.y-10), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
        }

        // 8. ��������
        imshow("����ʶ��", frame);
        int key = waitKey(10);
        if (key == 27) break; // ESC�˳�
        if (key == 32 && !faces.empty()) { // �ո����������
            string name;
            cout << "����������: ";
            cin >> name;
            vector<float> feature = extractFaceFeature(frame(faces[0]));
            if (faceDB.insertFace(name, feature)) {
                cout << "������������ɹ���" << endl;
            }
        }
    }

    return 0;
}
