//
// Created by Administrator on 2025/11/23.
//


#include "proto/test_req.pb.h"
#include "proto/test_resp.pb.h"
#include "protobuf/ProtoServer.h"

int main() {
    asio::io_context io_context;
    NETCPP::ProtoServer server(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8888));
    server.RegisterMessageHandler<demo::service::DemoRequest, demo::service::DemoResponse>(
        [](std::shared_ptr<demo::service::DemoRequest> msg, std::shared_ptr<demo::service::DemoResponse> resp) {
            std::cout << msg->Utf8DebugString() << std::endl;
            std::cout << "content: " << msg->content() << std::endl;
            std::cout << "type: " << msg->type() << std::endl;

            resp->set_result("这是 同步调用测试");
        }
    );
    server.start();
    io_context.run();
}
