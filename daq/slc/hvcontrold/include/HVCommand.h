#ifndef _Belle2_HVCommand_hh
#define _Belle2_HVCommand_hh

#include "base/Command.h"

namespace Belle2 {

  class State;

  class HVCommand : public Command {

  public:
    static const HVCommand LOAD;
    static const HVCommand SWITCHON;
    static const HVCommand SWITCHOFF;

  public:
    HVCommand() throw() {}
    HVCommand(const Enum& e) throw() : Command(e) {}
    HVCommand(const Command& cmd) throw() : Command(cmd) {}
    HVCommand(const HVCommand& cmd) throw() : Command(cmd) {}
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


