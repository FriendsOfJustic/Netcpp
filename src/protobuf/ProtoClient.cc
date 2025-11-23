//
// Created by Administrator on 2025/11/23.
//

#include "ProtoClient.h"

NETCPP::ProtoClient::ProtoClient(asio::io_context &io_context) : TcpClient(
    io_context) {
    SetReadCallback([this](ConnectionPtr ptr) {
        std::string reqId;
        requestor_.onResponse(std::move(ptr), reqId);
    });
}


