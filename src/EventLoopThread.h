//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_EVENTLOOPTHREAD_H_
#define NETCPP_SRC_EVENTLOOPTHREAD_H_
#include "asio.hpp"
#include "thread"
#include "CallbackInterface.h"
#include "iostream"

namespace NETCPP {
  class EventLoopThread {
  public:
    explicit EventLoopThread(std::string name)
      : name_(std::move(name)), io_context_(std::make_unique<asio::io_context>()) {
    }

    EventLoopThread(const EventLoopThread &) = delete;

    void SetReadCallback(std::function<void()> read_callback) {
      read_callback_ = std::move(read_callback);
    }

    void SetWriteCallback(std::function<void()> write_callback) {
      write_callback_ = std::move(write_callback);
    }

    void SetErrorCallback(std::function<void()> error_callback) {
      error_callback_ = std::move(error_callback);
    }

    void SetCloseCallback(std::function<void()> close_callback) {
      close_callback_ = std::move(close_callback);
    }

    void enableRead() {
      if (read_callback_ != nullptr) {
        read_callback_();
      }
    }

    void RunInLoop(std::function<void()> fn) {
      if (io_context_ != nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        asio::post(*io_context_, fn);
      }
    }

    asio::io_context &GetLoop() {
      return *io_context_;
    }

    void Stop() {
      if (io_context_ != nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        io_context_->stop();
      }
    }

    ~EventLoopThread() {
      if (thread_ != nullptr) {
        thread_->join();
      }
    }

    void start();

  private:
    std::function<void()> read_callback_;
    std::function<void()> write_callback_;
    std::function<void()> error_callback_;
    std::function<void()> close_callback_;
    std::mutex mutex_;
    std::string name_;
    std::unique_ptr<std::thread> thread_ = nullptr;
    std::unique_ptr<asio::io_context> io_context_ = nullptr;
  };

  class EventLoopThreadPool {
  public:
    void SetThreadNum(int thread_num) {
      thread_num_ = thread_num;
    }

    void start() {
      for (int i = 0; i < thread_num_; ++i) {
        auto thread = std::make_unique<EventLoopThread>(name_ + std::to_string(i));
        threads_.push_back(std::move(thread));
      }
      for (auto &thread: threads_) {
        thread->start();
      }
    }

    asio::io_context &GetNextLoop() {
      std::lock_guard<std::mutex> lock(mutex_);
      if (threads_.empty()) {
        throw std::runtime_error("EventLoopThreadPool is empty");
      }
      auto cnt = next_.fetch_add(1);
      return threads_[cnt % thread_num_]->GetLoop();
    }

    int threadNum() const { return thread_num_; }

  private:
    std::atomic<int> next_ = 0;
    int thread_num_ = 0;
    std::vector<std::unique_ptr<EventLoopThread> > threads_;
    std::mutex mutex_;
    std::string name_;
  };
} // NETCPP

#endif //NETCPP_SRC_EVENTLOOPTHREAD_H_
