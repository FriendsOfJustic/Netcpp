//
// Created by 13454 on 2025/7/24.
//

#ifndef NETCPP_SRC_TCPCLIENT_H_
#define NETCPP_SRC_TCPCLIENT_H_
#include "asio.hpp"
#include "Connection.h"
#include "string"

namespace NETCPP {
    class TcpClient : public std::enable_shared_from_this<TcpClient> {
    public:
        typedef std::shared_ptr<TcpClient> TcpClientPtr;

        using ReadCallback = std::function<void(ConnectionPtr ptr)>;

        TcpClient(asio::io_context &ctx) : ctx_(ctx) {
        }

        ~TcpClient() = default;

        bool Connect(const std::string &ip, uint16_t port);

        void AsyncConnect(const std::string &ip, uint16_t port,
                          const std::function<void(TcpClientPtr)> &func);

        void Close();

        bool IsConnected() { return connection_->GetSocket().is_open(); }

        void Disconnect();

        void Send(const std::string &data);

        void SetReadCallback(const ReadCallback &callback) {
            readCallback_ = callback;
        }

    protected:
        ReadCallback readCallback_;
        asio::io_context &ctx_;
        ConnectionPtr connection_;
    };
} // namespace NETCPP
#endif  // NETCPP_SRC_TCPCLIENT_H_
