#ifndef _Belle2_HVCommand_hh
#define _Belle2_HVCommand_hh

#include "daq/slc/base/Command.h"

namespace Belle2 {

  class State;

  class HVCommand : public Command {

  public:
    static const HVCommand CONFIGURE;
    static const HVCommand SAVE;
    static const HVCommand RAMPUP;
    static const HVCommand RAMPDOWN;
    static const HVCommand STANDBY;
    static const HVCommand STANDBY2;
    static const HVCommand STANDBY3;
    static const HVCommand PEAK;
    static const HVCommand RECOVER;
    static const HVCommand TURNON;
    static const HVCommand TURNOFF;

  public:
    HVCommand() throw() {}
    HVCommand(const Enum& e) throw() : Command(e) {}
    HVCommand(const Command& cmd) throw() : Command(cmd) {}
    HVCommand(const HVCommand& cmd) throw() : Command(cmd) {}
    HVCommand(const std::string& label) throw() { *this = label; }
    HVCommand(const char* label) throw() { *this = label; }
    ~HVCommand() throw() {}

  protected:
    HVCommand(int id, const char* label, const char* alias)
    throw() : Command(id, label, alias) {}

  public:
    //virtual int isAvailable(const State& state) const throw();
    State nextState() const throw();

  public:
    const HVCommand& operator=(const std::string& label) throw();
    const HVCommand& operator=(const char* label) throw();

  };

}

#endif


