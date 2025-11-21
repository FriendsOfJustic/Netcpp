//
// Created by 13454 on 2025/7/24.
//

#ifndef NETCPP_SRC_TCPCLIENT_H_
#define NETCPP_SRC_TCPCLIENT_H_
#include "asio.hpp"
#include "string"
namespace NETCPP {
class TcpClient : public std::enable_shared_from_this<TcpClient> {
 public:
  typedef std::shared_ptr<TcpClient> TcpClientPtr;

  TcpClient(asio::io_context& ctx) : ctx_(ctx) {}
  ~TcpClient() = default;
  bool Connect(const std::string& ip, uint16_t port);

  void AsyncConnect(const std::string& ip, uint16_t port,
                    const std::function<void(TcpClientPtr)>& func);
  void Close();
  bool IsConnected() { return socket_->is_open(); }
  void Disconnect();
  void Send(const std::string& data);
  void AsyncSend(const std::string& data,
                 const std::function<void(TcpClientPtr)>& func);

 private:
  asio::io_context& ctx_;
  std::unique_ptr<asio::ip::tcp::socket> socket_;
};

}  // namespace NETCPP
#endif  // NETCPP_SRC_TCPCLIENT_H_
