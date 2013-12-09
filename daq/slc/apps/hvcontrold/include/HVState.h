#ifndef _Belle2_HVState_hh
#define _Belle2_HVState_hh

#include "daq/slc/base/State.h"

namespace Belle2 {

  class HVState : public State {

  public:
    static const HVState OFF_STABLE_S;
    static const HVState OFF_ERROR_ES;
    static const HVState ON_STABLE_S;
    static const HVState ON_RAMPINGUP_TS;
    static const HVState ON_RAMPINGDOWN_TS;
    static const HVState ON_TRIP_ES;
    static const HVState ON_OCP_ES;
    static const HVState ON_OVP_ES;

  public:
    HVState() throw() : State(Enum::UNKNOWN) {}
    HVState(const Enum& num) throw() : State(num) {}
    HVState(const HVState& st) throw() : State(st) {}
    HVState(const State& st) throw() : State(st) {}
    HVState(const char* st) throw() { *this = st; }
    HVState(const std::string& st) throw() { *this = st; }
    HVState(int id) throw() { *this = id; }
    virtual ~HVState()  throw() {}

  protected:
    HVState(int id, const char* label, const char* alias)
    throw() : State(id, label, alias) {}

  public:
    bool isOn() const;
    bool isStable() const;
    bool isError() const;

  public:
    const HVState& operator=(const std::string& msg) throw();
    const HVState& operator=(const char* msg) throw();
    const HVState& operator=(int id) throw();

  };

}

#endif
