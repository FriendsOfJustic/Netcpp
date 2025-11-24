//
// Created by Administrator on 2025/11/22.
//

#ifndef NETCPP_DISPATCHER_H
#define NETCPP_DISPATCHER_H
#include <string>
#include <google/protobuf/message.h>

#include "Codec.h"
#include "Connection.h"

namespace NETCPP {
    class CallBack {
    public:
        virtual void onCall(ProtobufMessagePtr msg, ProtobufMessagePtr &resp) = 0;
    };


    template<typename Req, typename Resp>
    class CallBackT : public CallBack {
    public:
        using CallBackHandler = std::function<void(std::shared_ptr<Req> msg, std::shared_ptr<Resp> resp)>;


        CallBackT(CallBackHandler handler) : cb(handler) {
        }


        void onCall(ProtobufMessagePtr msg, ProtobufMessagePtr &resp) override {
            auto msg_ptr = std::dynamic_pointer_cast<Req>(msg);
            auto resp_ptr = std::make_shared<Resp>();
            if (msg_ptr) {
                cb(msg_ptr, resp_ptr);
            } else {
                spdlog::error("msg type {} not match", msg_ptr->GetTypeName());
            }
            resp = resp_ptr;
        }

    private:
        CallBackHandler cb;
    };

    class Dispatcher {
    public:
        void dispatch(ConnectionPtr ptr);

        template<class Req, class Resp>
        void RegisterMessageHandler(typename CallBackT<Req, Resp>::CallBackHandler cb);

    private:
        std::mutex mtx_;
        Codec codec_;
        std::unordered_map<std::string, std::shared_ptr<CallBack> > handlers_map_;
    };

    template<class Req, class Resp>
    void Dispatcher::RegisterMessageHandler(typename CallBackT<Req, Resp>::CallBackHandler cb) {
        auto name = Req::GetDescriptor()->full_name();
        std::lock_guard<std::mutex> lock(mtx_);
        auto cb_ptr = std::make_shared<CallBackT<Req, Resp> >(cb);
        handlers_map_[name] = cb_ptr;
    }
} // NETCPP

#endif //NETCPP_DISPATCHER_H
