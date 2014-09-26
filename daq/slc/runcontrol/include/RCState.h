#ifndef _Belle2_RCState_hh
#define _Belle2_RCState_hh

#include <daq/slc/nsm/NSMState.h>

namespace Belle2 {

  class RCState : public NSMState {

  public:
    static const RCState OFF_S;
    static const RCState NOTREADY_S;
    static const RCState READY_S;
    static const RCState RUNNING_S;
    static const RCState PAUSED_S;
    static const RCState LOADING_TS;
    static const RCState STARTING_TS;
    static const RCState STOPPING_TS;
    static const RCState BOOTING_TS;
    static const RCState ERROR_ES;
    static const RCState FATAL_ES;
    static const RCState RECOVERING_RS;
    static const RCState ABORTING_RS;

  public:
    RCState() throw() : NSMState(Enum::UNKNOWN) {}
    RCState(const Enum& e) throw() : NSMState(e) {}
    RCState(const NSMState& num) throw() : NSMState(num) {}
    RCState(const RCState& st) throw() : NSMState(st) {}
    RCState(const char* st) throw() { *this = st; }
    RCState(const std::string& st) throw() { *this = st; }
    RCState(int id) throw() { *this = id; }
    ~RCState()  throw() {}

  protected:
    RCState(int id, const char* label)
    throw() : NSMState(id, label) {}

  public:
    bool isStable() const throw() { return getId() > 1 && getId() <= 5; }
    bool isTransition() const throw() { return getId() > 5 && getId() <= 8; }
    bool isError() const throw() { return getId() > 9 && getId() <= 11; }
    bool isRecovering() const throw() { return getId() > 11 && getId() <= 13; }
    RCState next() const throw();

  public:
    const RCState& operator=(const std::string& msg) throw();
    const RCState& operator=(const char* msg) throw();
    const RCState& operator=(int id) throw();

  };

}

#endif
