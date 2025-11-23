//
// Created by Administrator on 2025/11/23.
//

#ifndef NETCPP_REQUESTOR_H
#define NETCPP_REQUESTOR_H
#include <future>
#include <string>
#include <unordered_map>

#include "Codec.h"
#include <google/protobuf/message.h>

#include "Connection.h"


namespace NETCPP {
    enum class RequestType {
        FUTURE, RespCALLBACK,
    };


    typedef std::function<void(const BaseMessagePtr &resp)> CallReqCallback;

    struct RequestDescriptor {
        std::string_view _id;
        RequestType _descriptor;
        std::promise<BaseMessagePtr> _promise;
        CallReqCallback _cb;
    };

    class Requestor {
    public:
        void Call(ConnectionPtr conn, BaseMessagePtr req, std::future<BaseMessagePtr> &future);

        void Call(ConnectionPtr conn, BaseMessagePtr req, BaseMessagePtr &resp);

        void Call(ConnectionPtr conn, BaseMessagePtr req, const CallReqCallback &callback);

        void onResponse(ConnectionPtr conn, std::string &reqId);

    private:
        std::mutex _mutex;

        Codec codec_;
        std::unordered_map<std::string, RequestDescriptor> req_map_;
    };
}


#endif //NETCPP_REQUESTOR_H
