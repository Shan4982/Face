#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

class ThreadChannel {
private:
    std::queue<int> dataQueue; // 数据队列
    std::mutex mtx; // 互斥锁
    std::condition_variable cv; // 条件变量
    bool isClosed = false; // 通道是否关闭

public:
    // 发送数据到通道
    void send(int data) {
        std::lock_guard<std::mutex> lock(mtx); // 加锁
        dataQueue.push(data); // 数据入队
        cv.notify_one(); // 通知一个等待的线程
    }
    // 从通道接收数据
    int receive() {
        std::unique_lock<std::mutex> lock(mtx); // 加锁
        cv.wait(lock, [this] { return !dataQueue.empty() || isClosed; }); // 等待数据或通道关闭
        if (isClosed && dataQueue.empty()) { // 通道关闭且队列为空
            return -1; // 返回结束标志
        }
        int data = dataQueue.front(); // 取出队首数据
        dataQueue.pop(); // 数据出队
        return data; // 返回数据
    }
};
