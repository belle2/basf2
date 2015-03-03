#ifndef _Belle2_RFCommand_hh
#define _Belle2_RFCommand_hh

#include "daq/slc/nsm/NSMCommand.h"

namespace Belle2 {

  class State;

  class RFCommand : public NSMCommand {

  public:
    static const RFCommand CONFIGURE;
    static const RFCommand UNCONFIGURE;
    static const RFCommand START;
    static const RFCommand STOP;
    static const RFCommand RESTART;
    static const RFCommand PAUSE;
    static const RFCommand RESUME;
    static const RFCommand STATUS;

  public:
    RFCommand() throw() {}
    RFCommand(const Enum& e) throw() : NSMCommand(e) {}
    RFCommand(const NSMCommand& cmd) throw() : NSMCommand(cmd) {}
    RFCommand(const RFCommand& cmd) throw() : NSMCommand(cmd) {}
    RFCommand(const char* label) throw() { *this = label; }
    ~RFCommand() throw() {}

  protected:
    RFCommand(int id, const char* label)
    throw() : NSMCommand(id, label) {}

  public:
    //virtual int isAvailable(const State& state) const throw();
    State nextState() const throw();

  public:
    const RFCommand& operator=(const std::string& label) throw();
    const RFCommand& operator=(const char* label) throw();

  };

}

#endif


