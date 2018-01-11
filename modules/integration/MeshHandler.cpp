#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "MeshHandler.hpp"

namespace ProtoMesh {

    MeshHandler::MeshHandler(cryptography::UUID deviceID, cryptography::asymmetric::KeyPair deviceKeys,
                             TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider)
            : transmissionHandler(std::move(transmissionHandler)), timeProvider(std::move(timeProvider)), network(deviceID, deviceKeys, timeProvider) {

    }

    MeshHandler MeshHandler::generateNew(TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider) {
        return MeshHandler(cryptography::UUID(),
                           cryptography::asymmetric::generateKeyPair(),
                           std::move(transmissionHandler),
                           std::move(timeProvider));
    }

#ifdef UNIT_TESTING
    SCENARIO("A device gets added", "[integration_test][module][integration]") {
        GIVEN("A mesh handler") {
            REL_TIME_PROV_T timeProvider(new DummyRelativeTimeProvider(0));
            TRANSMISSION_HANDLER_T transmissionHandler(new communication::transmission::NetworkStub());

            MeshHandler handler = MeshHandler::generateNew(transmissionHandler, timeProvider);

            class SomeDevDelegate : public DeviceHandlerDelegate {
            public:
                void didAddDevice(Device &dev) override {
                    dev.requestMetadata();
                    std::cout << "IT WORKED" << std::endl;
                };
            };

            handler.deviceDelegate = make_shared<SomeDevDelegate>();

            // ((DummyRelativeTimeProvider *) timeProvider.get())->turnTheClockBy(20000);
        }
    }
#endif // UNIT_TESTING
}