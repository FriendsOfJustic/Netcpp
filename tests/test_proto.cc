//
// Created by Administrator on 2025/11/22.
//


#include "./proto/test.pb.h"
#include <google/protobuf/message.h>

google::protobuf::Message *createMessage(const std::string &typeName) {
    // 1. 初始化返回值：默认NULL（类型名无效时返回）
    google::protobuf::Message *message = NULL;

    // 2. 从「生成的描述符池」中根据类型名查找消息的Descriptor
    const google::protobuf::Descriptor *descriptor =
            google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    // 3. 如果找到有效的Descriptor（类型名存在）
    if (descriptor) {
        // 4. 从「生成的消息工厂」中获取该Descriptor对应的消息原型（prototype）
        const google::protobuf::Message *prototype =
                google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);

        // 5. 如果原型存在（Descriptor可映射到具体Message类型）
        if (prototype) {
            // 6. 通过原型创建新的消息实例（空对象）
            message = prototype->New();
        }
    }

    // 7. 返回创建的实例（有效则为具体Message对象，无效则为NULL）
    return message;
}

int main() {
    demo::UserInfo info;

    info.set_age(1);
    info.add_hobbies("aaaa");
    auto serialize_as_string = info.SerializeAsString();
    auto desc = info.GetDescriptor();

    std::cout << "field count: " << desc->field_count() << std::endl;
    std::cout << "message name: " << desc->full_name() << std::endl;
    for (int i = 0; i < desc->field_count(); i++) {
        auto field = desc->field(i);
        std::cout << "field name: " << field->name() << std::endl;
    }

    auto s = createMessage(desc->full_name());
    auto msg2 = dynamic_cast<demo::UserInfo *>(s);
    if (msg2) {
        std::cout << "create message success" << std::endl;
        msg2->set_age(1);
    } else {
        std::cout << "create message failed" << std::endl;
    }
}
