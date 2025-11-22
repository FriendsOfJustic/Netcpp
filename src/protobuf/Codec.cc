//
// Created by Administrator on 2025/11/22.
//

#include "Codec.h"
#include <spdlog/spdlog.h>

namespace NETCPP {
    const char *CodecErrorCategory::name() const noexcept {
        return "CodecErrorCategory";
    }

    std::string CodecErrorCategory::message(int ev) const {
        switch (static_cast<CodecError>(ev)) {
            case CodecError::InvalidMessage:
                return "InvalidMessage";
        }
    }

    BaseMessagePtr Codec::createMessage(const std::string &typeName) {
        // 1. 初始化返回值：默认NULL（类型名无效时返回）
        BaseMessagePtr message = nullptr;

        // 2. 从「生成的描述符池」中根据类型名查找消息的Descriptor
        const google::protobuf::Descriptor *descriptor =
                google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
        // 3. 如果找到有效的Descriptor（类型名存在）
        if (descriptor) {
            spdlog::debug("create message: {}", descriptor->full_name());
            // 4. 从「生成的消息工厂」中获取该Descriptor对应的消息原型（prototype）
            const google::protobuf::Message *prototype =
                    google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);

            // 5. 如果原型存在（Descriptor可映射到具体Message类型）
            if (prototype) {
                // 6. 通过原型创建新的消息实例（空对象）
                message = std::shared_ptr<google::protobuf::Message>(prototype->New());
            }
        }

        // 7. 返回创建的实例（有效则为具体Message对象，无效则为NULL）
        return message;
    }

    bool Codec::deSerialize(Buffer &buffer, BaseMessagePtr &message) {
        int32_t msg_len = buffer.peekInt32();
        if (msg_len <= 0 || msg_len > buffer.readableBytes()) {
            spdlog::error("invalid message length: {}", msg_len);
            return false;
        }
    }
} // NETCPP
