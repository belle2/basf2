#ifndef _B2DAQ_RCState_hh
#define _B2DAQ_RCState_hh

#include <node/State.hh>

namespace B2DAQ {

  class RCState : public State {

  public:
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
    RCState() throw() : State(Enum::UNKNOWN) {}
    RCState(const Enum& num) throw() : State(num) {}
    RCState(const RCState& st) throw() : State(st) {}
    RCState(const char* st) throw() { *this = st; }
    RCState(const std::string& st) throw() { *this = st; }
    RCState(int id) throw() { *this = id; }
    ~RCState()  throw() {}

  private:
    RCState(int id, const char* label, const char* alias)
      throw() : State(id, label, alias) {}
 
  public:
    bool isStable() const throw() { return _id > 100 && _id < 200; }
    bool isTransaction() const throw() { return _id > 200 && _id < 300; }
    bool isError() const throw() { return _id > 300 && _id < 400; }
    bool isRecovering() const throw() { return _id > 400 && _id < 500; }
    RCState next() const throw();

  public:
    const RCState& operator=(const std::string& msg) throw();
    const RCState& operator=(const char* msg) throw();
    const RCState& operator=(int id) throw();

  };

}

#endif
