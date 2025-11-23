//
// Created by Administrator on 2025/11/23.
//

#include "Requestor.h"

#include "UUID.h"


using namespace NETCPP;

void Requestor::Call(ConnectionPtr conn, BaseMessagePtr req, std::future<BaseMessagePtr> &future) {
    RequestDescriptor desc;
    auto id = UUID::GenerateUUID();
    desc._id = id;
    desc._descriptor = RequestType::FUTURE;
    desc._promise = std::promise<BaseMessagePtr>();
    future = desc._promise.get_future();
    req_map_[id] = std::move(desc);
    codec_.serialize(req, conn->WriteBuffer(), id);
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
    codec_.serialize(req, conn->WriteBuffer(), id);
    conn->Write(nullptr, 0);
}


void Requestor::onResponse(ConnectionPtr conn, std::string &reqId) {
    BaseMessagePtr resp;
    codec_.deSerialize(conn->ReadBuffer(), resp, reqId);
    auto it = req_map_.find(reqId);
    if (it == req_map_.end()) {
        spdlog::error("response not found: {}", reqId);
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
