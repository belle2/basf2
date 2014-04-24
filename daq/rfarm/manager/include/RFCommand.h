#ifndef _Belle2_RFCommand_hh
#define _Belle2_RFCommand_hh

#include "daq/slc/nsm/NSMCommand.h"

namespace Belle2 {

  class State;

  class RFCommand : public NSMCommand {

  public:
    static const RFCommand RF_CONFIGURE;
    static const RFCommand RF_UNCONFIGURE;
    static const RFCommand RF_START;
    static const RFCommand RF_STOP;
    static const RFCommand RF_RESTART;
    static const RFCommand RF_PAUSE;
    static const RFCommand RF_RESUME;
    static const RFCommand RF_STATUS;

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


