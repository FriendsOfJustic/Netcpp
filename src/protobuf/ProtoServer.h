//
// Created by Administrator on 2025/11/23.
//

#ifndef NETCPP_PROTOSERVER_H
#define NETCPP_PROTOSERVER_H
#include "Dispatcher.h"
#include "TcpServer.h"

namespace NETCPP {
    class ProtoServer : public TcpServer, public Dispatcher {
    public:
        ProtoServer(asio::io_context &io_context, const asio::ip::tcp::endpoint &endpoint);

    private:
    };
} // NETCPP

#endif //NETCPP_PROTOSERVER_H
