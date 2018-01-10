#ifndef PROTOMESH_DEVICEDELEGATE_HPP
#define PROTOMESH_DEVICEDELEGATE_HPP

#import <memory>

namespace ProtoMesh {

#define DEVICE_HANDLER_DELEGATE_T shared_ptr<DeviceHandlerDelegate>

    class DeviceDelegate {
    public:
        DeviceDelegate() = default;
        ~DeviceDelegate() = default;

        virtual void didAddMetadata() {}; // TODO Add Metadata class as parameter
    };

}

#endif //PROTOMESH_DEVICEDELEGATE_HPP
