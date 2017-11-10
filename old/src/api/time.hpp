//
// Created by themegatb on 5/28/17.
//

#ifndef PROTOMESH_TIME_HPP
#define PROTOMESH_TIME_HPP

#define REL_TIME_PROV_T std::shared_ptr<RelativeTimeProvider>

class RelativeTimeProvider {
public:
    virtual long millis()= 0;
};

#ifdef UNIT_TESTING
class DummyRelativeTimeProvider : public RelativeTimeProvider {
    long time = 0;
public:
    inline long millis() { return this.time; };

    inline void turnTheClockBy(long duration) { this.time += duration; }
};
#endif

#endif //PROTOMESH_TIME_HPP
