#ifndef _Belle2_RCState_hh
#define _Belle2_RCState_hh

#include <daq/slc/nsm/NSMState.h>

namespace Belle2 {

  class RCState : public NSMState {

  public:
    static const RCState OFF_S;
    static const RCState INITIAL_S;
    static const RCState CONFIGURED_S;
    static const RCState READY_S;
    static const RCState RUNNING_S;
    static const RCState PAUSED_S;
    static const RCState BOOTING_TS;
    static const RCState LOADING_TS;
    static const RCState STARTING_TS;
    static const RCState STOPPING_TS;
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
    bool isStable() const throw() { return _id > 1 && _id < 7; }
    bool isTransition() const throw() { return _id > 6 && _id < 11; }
    bool isError() const throw() { return _id > 10 && _id < 13; }
    bool isRecovering() const throw() { return _id > 12 && _id < 15; }
    RCState next() const throw();

  public:
    const RCState& operator=(const std::string& msg) throw();
    const RCState& operator=(const char* msg) throw();
    const RCState& operator=(int id) throw();
    inline const RCState& operator=(const RCState& e) throw() {
      _id = e._id;
      _label = e._label;
      return *this;
    }

  };

}

#endif
