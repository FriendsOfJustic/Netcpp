//
// Created by Administrator on 2025/11/23.
//

#ifndef NETCPP_PROTOCLIENT_H
#define NETCPP_PROTOCLIENT_H
#include "Codec.h"
#include "Dispatcher.h"
#include "Requestor.h"
#include "TcpClient.h"


namespace NETCPP {
    class ProtoClient : public NETCPP::TcpClient {
    public:
        ProtoClient(asio::io_context &io_context);


        template<typename Req, typename Resp>
        void Call(std::shared_ptr<Req> req, std::shared_ptr<Resp> &resp);


        template<typename Req, typename Resp>
        void Call(std::shared_ptr<Req> req, std::future<std::shared_ptr<Resp> > &future);

    private:
        Requestor requestor_;
    };

    template<typename Req, typename Resp>
    void ProtoClient::Call(std::shared_ptr<Req> req, std::shared_ptr<Resp> &resp) {
        auto castReq = std::dynamic_pointer_cast<google::protobuf::Message>(req);
        if (!castReq) {
            throw std::runtime_error("Req is not a proto message");
        }
        BaseMessagePtr rawResp;
        requestor_.Call(connection_, castReq, rawResp);
        resp = std::dynamic_pointer_cast<Resp>(rawResp);
        if (!resp) {
            throw std::runtime_error("Resp is not a proto message");
        }
    }

    template<typename Req, typename Resp>
    void ProtoClient::Call(std::shared_ptr<Req> req, std::future<std::shared_ptr<Resp> > &future) {
        auto castReq = std::dynamic_pointer_cast<google::protobuf::Message>(req);
        if (!castReq) {
            throw std::runtime_error("Req is not a proto message");
        }
        BaseMessagePtr rawResp;
        auto cbfunc = [&future](BaseMessagePtr resp) {
            auto castResp = std::dynamic_pointer_cast<Resp>(resp);
            if (!castResp) {
                throw std::runtime_error("Resp is not a proto message");
            }
            future.set_value(castResp);
        };
        requestor_.Call(connection_, castReq, cbfunc);
    }
}


#endif //NETCPP_PROTOCLIENT_H
