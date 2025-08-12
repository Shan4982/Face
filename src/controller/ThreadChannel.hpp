#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

class ThreadChannel {
private:
    std::queue<double> dataQueue; // ���ݶ���
    std::mutex mtx; // ������
    std::condition_variable cv; // ��������
    bool isClosed = false; // ͨ���Ƿ�ر�
    bool if_end = false;
public:
    // �������ݵ�ͨ��
    ~ThreadChannel() {
        close(); // ȷ��ͨ��������ʱ�ر�
    }

    void send(double data) {
        std::lock_guard<std::mutex> lock(mtx); // ����
        dataQueue.push(data); // �������
        cv.notify_one(); // ֪ͨһ���ȴ����߳�
    }
    // ��ͨ����������
    double receive() {
        std::unique_lock<std::mutex> lock(mtx); // ����
        cv.wait(lock, [this] { return !dataQueue.empty() || isClosed; }); // �ȴ����ݻ�ͨ���ر�
        if (isClosed && dataQueue.empty()) { // ͨ���ر��Ҷ���Ϊ��
            return double(); // ���ؽ�����־
        }
        double data = dataQueue.front(); // ȡ����������
        dataQueue.pop(); // ���ݳ���
        return data; // ��������
    }

    void close() {
    std::lock_guard<std::mutex> lock(mtx);
    isClosed = true;
    cv.notify_all();
}

    // size_t queue_size() const{ // ����
    //     std::lock_guard<std::mutex> lock(mtx);
    //     return dataQueue.size();
    // }
};

