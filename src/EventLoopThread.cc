//
// Created by 13454 on 2025/7/22.
//

#include "EventLoopThread.h"

namespace NETCPP {
void EventLoopThread::start() {
  thread_ = std::make_unique<std::thread>([this]() {
    auto worker = asio::make_work_guard(*io_context_);
    io_context_->run();
  });
}
} // NETCPP