//
// Created by Administrator on 2025/11/22.
//

#ifndef NETCPP_DISPATCHER_H
#define NETCPP_DISPATCHER_H
#include <string>

#include "Connection.h"

namespace NETCPP {
    class Dispatcher {
    public:
        void onMessage(ConnectionPtr ptr);

        template<class T>
        void RegisterMessageHandler(const std::string &name);

    private:
    };
} // NETCPP

#endif //NETCPP_DISPATCHER_H
