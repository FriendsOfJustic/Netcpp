//
// Created by Administrator on 2025/11/23.
//

#include "ProtoServer.h"

namespace NETCPP {
    ProtoServer::ProtoServer(asio::io_context &io_context, const asio::ip::tcp::endpoint &endpoint) : TcpServer(
        io_context, endpoint) {
        SetReadCallback([this](ConnectionPtr ptr) { dispatch(std::move(ptr)); });
    }
} // NETCPP
