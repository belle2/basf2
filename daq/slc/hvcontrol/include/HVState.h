#ifndef _Belle2_HVState_hh
#define _Belle2_HVState_hh

#include "daq/slc/nsm/NSMState.h"

namespace Belle2 {

  class HVState : public NSMState {

  public:
    static const HVState OFF_S;
    static const HVState STABLE_S;
    static const HVState STANDBY_S;
    static const HVState SHOULDER_S;
    static const HVState PEAK_S;
    static const HVState TRANSITION_TS;
    static const HVState RAMPINGUP_TS;
    static const HVState RAMPINGDOWN_TS;
    static const HVState TURNINGOFF_TS;
    static const HVState TURNINGON_TS;
    static const HVState RECOVERING_TS;
    static const HVState TRIP_ES;
    static const HVState OVER_CURRENT_ES;
    static const HVState OVER_VOLTAGE_ES;
    static const HVState ERROR_ES;

  public:
    HVState() throw() : NSMState(Enum::UNKNOWN) {}
    HVState(const Enum& num) throw() : NSMState(num) {}
    HVState(const NSMState& st) throw() : NSMState(st) {}
    HVState(const HVState& st) throw() : NSMState(st) {}
    HVState(const char* st) throw() { *this = st; }
    HVState(const std::string& st) throw() { *this = st; }
    HVState(int id) throw() { *this = id; }
    virtual ~HVState()  throw() {}

  protected:
    HVState(int id, const char* label)
    throw() : NSMState(id, label) {}

  public:
    bool isOff() const;
    bool isOn() const;
    bool isStable() const { return isOn() || isOff(); }
    bool isTransition() const;
    bool isError() const;

  public:
    const HVState& operator=(const std::string& msg) throw();
    const HVState& operator=(const char* msg) throw();
    const HVState& operator=(int id) throw();

  };

}

#endif
