#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include <memory>
#include <chrono>

using namespace std;
using namespace cv;

// 基于智能指针的零拷贝数据传输通道
template <typename T>
class ThreadChannel {
private:
    std::queue<std::shared_ptr<T>> dataQueue; // 数据队列
    std::mutex mtx; // 互斥锁
    std::condition_variable cv; // 条件变量
    bool isClosed = false; // 通道是否关闭

public:
    ~ThreadChannel() {
        close(); // 确保通道在析构时关闭
    }

    // 发送数据到通道 (零拷贝，传入智能指针)
    void send(std::shared_ptr<T> data) {
        std::lock_guard<std::mutex> lock(mtx); // 加锁
        
        // 性能优化：限制队列长度，防止处理不及时导致画面卡顿/内存溢出
        // 如果积压超过 30 帧（约 1-2 秒数据），则丢弃最旧的数据
        if (dataQueue.size() > 30) {
            dataQueue.pop();
        }
        
        dataQueue.push(data); // 数据入队
        cv.notify_one(); // 通知一个等待的线程
    }

    // 从通道接收数据 (带有死锁检测/超时机制)
    std::shared_ptr<T> receive(int timeout_ms = 5000) {
        std::unique_lock<std::mutex> lock(mtx); // 加锁
        // 使用 wait_for 实现简单的死锁检测
        bool success = cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                                   [this] { return !dataQueue.empty() || isClosed; });
        
        if (!success) {
            cerr << "警告: ThreadChannel 接收超时，可能发生死锁或上游处理停滞！" << endl;
            return nullptr;
        }

        if (isClosed && dataQueue.empty()) { // 通道关闭且队列为空
            return nullptr; // 返回结束标志
        }

        std::shared_ptr<T> data = dataQueue.front(); // 取出队首数据
        dataQueue.pop(); // 数据出队
        return data; // 返回数据
    }

    void close() {
        std::lock_guard<std::mutex> lock(mtx);
        isClosed = true;
        cv.notify_all();
    }
};


