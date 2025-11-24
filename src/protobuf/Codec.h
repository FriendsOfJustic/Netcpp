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


    class Message {
    public:
        std::shared_ptr<google::protobuf::Message> message;
        std::string NETCPP_ID;
    };


    using BaseMessagePtr = std::shared_ptr<Message>;

    using ProtobufMessagePtr = std::shared_ptr<google::protobuf::Message>;


    /**
     * | len | reqIdLen | reqId | typeNameLen | typeName | message | checksum |
     * len: 4字节，小端存储，消息总长度（包含所有字段）
     * reqIdLen: 1字节，存储请求ID长度（固定为16字节）
     * reqId: 16字节，存储请求ID（UUID）
     * typeNameLen: 1字节，存储类型名长度
     * typeName: 变长字段，存储消息类型名（UTF-8编码）
     * message: 变长字段，存储序列化后的消息内容（二进制格式）
     * checksum: 4字节，小端存储，校验和（简单的XOR校验）
     */
    class Codec {
    public:
        static ProtobufMessagePtr createMessage(const std::string &typeName);


        bool deSerialize(Buffer &buffer, BaseMessagePtr &message);

        void serialize(BaseMessagePtr message, Buffer &buffer);


        bool deSerialize(std::string &buffer, BaseMessagePtr &message);

        void serialize(BaseMessagePtr message, std::string &buffer);

    private:
        const static int32_t FieldLenSize = 4;
        const static int32_t FieldReqIdLenSize = 4;
        const static int32_t FieldTypeNameLenSize = 4;
        const static int32_t FieldCheckSumLenSize = 4;
    };
} // NETCPP

#endif //NETCPP_CODEC_H
