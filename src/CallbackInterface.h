//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_CALLBACKINTERFACE_H_
#define NETCPP_SRC_CALLBACKINTERFACE_H_

class ReadCallback {
 public:
  virtual void onRead() = 0;
};

class WriteCallback {
 public:
  virtual void onWrite() = 0;
};

class ErrorCallback {
 public:
  virtual void onError() = 0;
};

class CloseCallback {
 public:
  virtual void onClose() = 0;
};

#endif //NETCPP_SRC_CALLBACKINTERFACE_H_
