#ifndef _Belle2_HVState_hh
#define _Belle2_HVState_hh

#include "base/State.h"

namespace Belle2 {

  class HVState : public State {

  public:
    static const HVState HVOFF_S;
    static const HVState STANDBY1_S;
    static const HVState STANDBY2_S;
    static const HVState PEAK_S;

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
    virtual State next() const throw();

  public:
    const HVState& operator=(const std::string& msg) throw();
    const HVState& operator=(const char* msg) throw();
    const HVState& operator=(int id) throw();

  };

}

#endif
