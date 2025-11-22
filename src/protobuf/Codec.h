//
// Created by Administrator on 2025/11/22.
//

#ifndef NETCPP_CODEC_H
#define NETCPP_CODEC_H

#include <memory>
#include <system_error>
#include <google/protobuf/message.h>
#include <Buffer.h>

namespace NETCPP {
    enum class CodecError {
        InvalidMessage = 1,
    };

    class CodecErrorCategory : public std::error_category {
    public:
        const char *name() const noexcept override;

        std::string message(int ev) const override;
    };

    using BaseMessagePtr = std::shared_ptr<google::protobuf::Message>;


    /**
     * | len  | typeNameLen | typeName | message | checksum |
     * len: 4字节，小端存储，消息总长度（包含所有字段）
     * typeNameLen: 1字节，存储类型名长度
     * typeName: 变长字段，存储消息类型名（UTF-8编码）
     * message: 变长字段，存储序列化后的消息内容（二进制格式）
     * checksum: 4字节，小端存储，校验和（简单的XOR校验）
     */
    class Codec {
    public:
        static BaseMessagePtr createMessage(const std::string &typeName);

        static uint32_t crc32_table[256];

        // 初始化CRC32表（基于IEEE标准）
        static void init_crc32_table();

        static uint32_t crc32(const std::string &str);

        bool deSerialize(Buffer &buffer, BaseMessagePtr &message);

        void serialize(BaseMessagePtr message, Buffer &buffer);

    private
    :
        const static int32_t FieldLenSize = 4;
        const static int32_t FieldTypeNameLenSize = 4;
        const static int32_t FieldCheckSumLenSize = 4;
    };
} // NETCPP

#endif //NETCPP_CODEC_H
