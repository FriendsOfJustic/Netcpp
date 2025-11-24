//
// Created by Administrator on 2025/11/22.
//

#include "Dispatcher.h"

namespace NETCPP {
    void Dispatcher::dispatch(ConnectionPtr ptr) {
        BaseMessagePtr message = std::make_shared<Message>();
        while (codec_.deSerialize(ptr->ReadBuffer(), message)) {
            auto it = handlers_map_.find(message->message->GetTypeName());
            if (it != handlers_map_.end()) {
                try {
                    auto resp = std::make_shared<Message>();
                    resp->NETCPP_ID = message->NETCPP_ID;
                    it->second->onCall(message->message, resp->message);
                    ptr->SendMsg(resp);
                } catch (const std::exception &e) {
                    spdlog::error("call back error: {}", e.what());
                }
            } else {
                spdlog::error("revic message type {} not register", message->message->GetTypeName());
            }
        }
    }
} // NETCPP
