//
// Created by Administrator on 2025/11/23.
//


#include "TcpClient.h"
#include "proto/test_req.pb.h"
#include "proto/test_resp.pb.h"
#include "protobuf/Codec.h"
#include "protobuf/ProtoClient.h"

int main() {
    asio::io_context io_context;

    NETCPP::ProtoClient client(io_context);
    client.Connect("127.0.0.1", 8888);
    auto req = std::make_shared<demo::service::DemoRequest>();
    auto resp = std::make_shared<demo::service::DemoResponse>();
    req->set_content("这是 同步调用测试");
    req->set_type(demo::service::REQUEST_TYPE_QUERY);
    client.Call<demo::service::DemoRequest, demo::service::DemoResponse>(req, resp);
    spdlog::info("收到响应: {}", resp->DebugString());
}
