//
// Created by Administrator on 2025/11/22.
//

#include "Codec.h"
#include <spdlog/spdlog.h>

namespace NETCPP {
    uint32_t Codec::crc32_table[256];

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

    void Codec::init_crc32_table() {
        const uint32_t poly = 0x04C11DB7; // CRC32-IEEE生成多项式
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
            }
            crc32_table[i] = crc;
        }
    }

    uint32_t Codec::crc32(const std::string &str) {
        static bool table_init = false;
        if (!table_init) {
            init_crc32_table();
            table_init = true;
        }

        uint32_t crc = 0xFFFFFFFF; // 初始值
        for (unsigned char c: str) {
            // 输入字节反转 + 查表异或
            crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ c];
        }
        return ~crc; // 最终异或0xFFFFFFFF（等价于~）
    }

    bool Codec::deSerialize(Buffer &buffer, BaseMessagePtr &message) {
        // 如果不满4字节直接返回
        if (buffer.size() < Codec::FieldLenSize) {
            return false;
        }
        int32_t msg_len = buffer.peekInt32();
        if (msg_len <= 0 || msg_len + Codec::FieldLenSize < buffer.size()) {
            spdlog::error("invalid message length: {}", msg_len);
            return false;
        }
        // 如果够一个消息的长度
        // 1. 从缓冲区读取消息体（跳过长度字段）
        std::string typeName = buffer.retrieveString(Codec::FieldLenSize);
        message = createMessage(typeName);
        if (!message) {
            spdlog::error("create message failed: {}", typeName);
            return false;
        }
        auto msg_str = buffer.retrieveString(msg_len - Codec::FieldTypeNameLenSize - typeName.size());
        // 2. 解析消息体字符串到指定的消息对象（通过引用传递）
        if (!message->ParseFromString(msg_str)) {
            spdlog::error("parse message failed");
            return false;
        }
        return true;
    }

    void Codec::serialize(BaseMessagePtr message, Buffer &buffer) {
        int32_t total_size = 0;
        auto typeName = message->GetDescriptor()->full_name();
        int32_t typeNameSize = typeName.size();
        auto binary = message->SerializeAsString();
        auto crc = Codec::crc32(binary);
        total_size = Codec::FieldTypeNameLenSize + typeName.size() + binary.size() +
                     Codec::FieldLenSize;
        buffer.writeInt32(total_size);
        buffer.writeInt32(typeNameSize);
        buffer.write(typeName.c_str(), typeName.size());
        buffer.write(binary.c_str(), binary.size());
        buffer.writeInt32(crc);
    }
} // NETCPP
