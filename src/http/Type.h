//
// Created by 13454 on 2025/7/23.
//

#ifndef NETCPP_SRC_HTTP_TYPE_H_
#define NETCPP_SRC_HTTP_TYPE_H_

#include <functional>
#include <string>
#include <unordered_map>
#include "HttpRequest.h"
#include "HttpResponse.h"
namespace NETCPP {

enum class HTTP_METHOD {
  GET,
  POST,
  PUT,
  DELETE_,
  HEAD,
  OPTIONS,
  CONNECT,
  TRACE,
  PATCH,
  OTHERS
};

extern std::unordered_map<std::string, HTTP_METHOD> STRING2HTTP_METHOD;

extern std::unordered_map<HTTP_METHOD, std::string> HTTP_METHOD2STRING;
}

#endif //NETCPP_SRC_HTTP_TYPE_H_
