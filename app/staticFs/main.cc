//
// Created by Administrator on 2025/11/26.
//

#include "http/HttpServer.h"

#include <filesystem>

int main(int argc, char *argv[]) {
    asio::io_service io_service;
    NETCPP::HttpServer server(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), atoi(argv[1])));
    auto baseDir = std::filesystem::path(argv[2]);
    server.GET("/", [baseDir](NETCPP::HttpContext &ctx) {
        auto &req = ctx.GetRequest();
        auto &resp = ctx.GetResponse();
        auto path = baseDir / req.GetPath();
        if (std::filesystem::exists(path)) {
            resp.SendFile(path);
        } else {
            resp.SetStatusCode(404);
            resp.SetBody("404 Not Found");
        }
    });
    server.SetThreadNum(4);
    server.start();
    io_service.run();
}
