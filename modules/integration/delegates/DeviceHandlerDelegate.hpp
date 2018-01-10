#ifndef PROTOMESH_DEVICEHANDLERDELEGATE_HPP
#define PROTOMESH_DEVICEHANDLERDELEGATE_HPP

#include <memory>

namespace ProtoMesh {

#define DEVICE_HANDLER_DELEGATE_T shared_ptr<DeviceHandlerDelegate>

    class DeviceHandlerDelegate {
    public:
        DeviceHandlerDelegate() = default;
        ~DeviceHandlerDelegate() = default;

        virtual void didAddDevice(Device &dev) {};
        virtual void didRemoveDevice(Device &dev) {};
    };

}


#endif //PROTOMESH_DEVICEHANDLERDELEGATE_HPP
