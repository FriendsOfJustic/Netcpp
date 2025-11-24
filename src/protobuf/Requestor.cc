//
// Created by Administrator on 2025/11/23.
//

#include "Requestor.h"

#include "UUID.h"


using namespace NETCPP;

void Requestor::Call(ConnectionPtr conn, BaseMessagePtr req, std::future<BaseMessagePtr> &future) {
    // RequestDescriptor desc;
    auto id = UUID::GenerateUUID();
    auto [it, inserted] = req_map_.emplace(id, RequestDescriptor());
    if (!inserted) {
        spdlog::error("request id {} already exists", id);
        return;
    }
    auto &desc = it->second;
    desc._id = it->first;
    desc._descriptor = RequestType::FUTURE;
    desc._promise = std::promise<BaseMessagePtr>();
    req->NETCPP_ID = id;
    future = desc._promise.get_future();

    codec_.serialize(req, conn->WriteBuffer());
    conn->Write(nullptr, 0);
}

void Requestor::Call(ConnectionPtr conn, BaseMessagePtr req, BaseMessagePtr &resp) {
    std::future<BaseMessagePtr> future;
    Call(std::move(conn), std::move(req), future);

    resp = future.get();
}

void Requestor::Call(ConnectionPtr conn, BaseMessagePtr req, const CallReqCallback &callback) {
    RequestDescriptor desc;
    auto id = UUID::GenerateUUID();
    desc._id = id;
    desc._descriptor = RequestType::RespCALLBACK;
    desc._cb = callback;
    req_map_[id] = std::move(desc);
    codec_.serialize(req, conn->WriteBuffer());
    conn->Write(nullptr, 0);
}


void Requestor::onResponse(ConnectionPtr conn) {
    BaseMessagePtr resp = std::make_shared<Message>();
    codec_.deSerialize(conn->ReadBuffer(), resp);
    auto it = req_map_.find(resp->NETCPP_ID);
    if (it == req_map_.end()) {
        spdlog::error("response not found: {}", resp->NETCPP_ID);
        return;
    }
    switch (it->second._descriptor) {
        case RequestType::FUTURE:
            it->second._promise.set_value(resp);
            break;
        case RequestType::RespCALLBACK:
            it->second._cb(resp);
            break;
        default:
            spdlog::error("unknown request type: {}", (int) it->second._descriptor);
            break;
    }
    req_map_.erase(it);
}
