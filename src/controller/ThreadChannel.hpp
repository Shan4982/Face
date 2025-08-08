#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

class ThreadChannel {
private:
    std::queue<int> dataQueue; // ���ݶ���
    std::mutex mtx; // ������
    std::condition_variable cv; // ��������
    bool isClosed = false; // ͨ���Ƿ�ر�

public:
    // �������ݵ�ͨ��
    void send(int data) {
        std::lock_guard<std::mutex> lock(mtx); // ����
        dataQueue.push(data); // �������
        cv.notify_one(); // ֪ͨһ���ȴ����߳�
    }
    // ��ͨ����������
    int receive() {
        std::unique_lock<std::mutex> lock(mtx); // ����
        cv.wait(lock, [this] { return !dataQueue.empty() || isClosed; }); // �ȴ����ݻ�ͨ���ر�
        if (isClosed && dataQueue.empty()) { // ͨ���ر��Ҷ���Ϊ��
            return -1; // ���ؽ�����־
        }
        int data = dataQueue.front(); // ȡ����������
        dataQueue.pop(); // ���ݳ���
        return data; // ��������
    }
};
