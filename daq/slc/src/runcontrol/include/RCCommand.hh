#ifndef _B2DAQ_RCCommand_hh
#define _B2DAQ_RCCommand_hh

#include <node/Command.hh>

namespace B2DAQ {

  class State;
  
  class RCCommand : public Command {

  public:
    static const RCCommand UNKNOWN;
    static const RCCommand BOOT;
    static const RCCommand REBOOT;
    static const RCCommand LOAD;
    static const RCCommand RELOAD;
    static const RCCommand START;
    static const RCCommand STOP;
    static const RCCommand RESUME;
    static const RCCommand PAUSE;
    static const RCCommand RECOVER;
    static const RCCommand ABORT;
    static const RCCommand STATECHECK;
    static const RCCommand STATE;
    static const RCCommand SET;

  public:
    RCCommand() throw() {}
    RCCommand(const Enum& e) throw() : Command(e) {}
    RCCommand(const Command& cmd) throw() : Command(cmd) {}
    RCCommand(const RCCommand& cmd) throw() : Command(cmd) {}
    RCCommand(const char* label) throw() { *this = label; }
    ~RCCommand() throw() {}

  private:
    RCCommand(int id, const char* label, const char* alias)
      throw() : Command(id, label, alias) {}

  public:
    int isAvailable(const State& state) const throw();
    State nextState() const throw();

  public:
    const RCCommand& operator=(const std::string& label) throw();
    const RCCommand& operator=(const char* label) throw();

  };

}

#endif
