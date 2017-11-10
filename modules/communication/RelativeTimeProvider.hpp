#ifndef PROTOMESH_TIME_HPP
#define PROTOMESH_TIME_HPP

#include <memory>

namespace ProtoMesh {

#define REL_TIME_PROV_T std::shared_ptr<RelativeTimeProvider>

    class RelativeTimeProvider {
    public:
        virtual long millis()= 0;
    };

#ifdef UNIT_TESTING

    class DummyRelativeTimeProvider : public RelativeTimeProvider {
        long time = 0;
    public:
        explicit DummyRelativeTimeProvider(long time) : time(time) {};

        inline long millis() override { return this->time; };

        inline void turnTheClockBy(long duration) { this->time += duration; }
    };

#endif

};

#endif //PROTOMESH_TIME_HPP