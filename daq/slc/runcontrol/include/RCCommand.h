#ifndef _Belle2_RCCommand_h
#define _Belle2_RCCommand_h

#include <daq/slc/nsm/NSMCommand.h>

#include "daq/slc/runcontrol/RCState.h"

namespace Belle2 {

  class RCCommand : public NSMCommand {

  public:
    static const RCCommand CONFIGURE;
    static const RCCommand BOOT;
    static const RCCommand LOAD;
    static const RCCommand START;
    static const RCCommand STOP;
    static const RCCommand RECOVER;
    static const RCCommand RESUME;
    static const RCCommand PAUSE;
    static const RCCommand ABORT;
    static const RCCommand STATUS;

  public:
    RCCommand() throw() {}
    RCCommand(const Enum& e) throw() : NSMCommand(e) {}
    RCCommand(const NSMCommand& e) throw() : NSMCommand(e) {}
    RCCommand(const RCCommand& cmd) throw() : NSMCommand(cmd) {}
    RCCommand(const std::string& label) throw() { *this = label; }
    RCCommand(const char* label) throw() { *this = label; }
    RCCommand(int id) throw() { *this = id; }
    ~RCCommand() throw() {}

  protected:
    RCCommand(int id, const char* label)
    throw() : NSMCommand(id, label) {}

  public:
    int isAvailable(const RCState& state) const throw();
    RCState nextState() const throw();
    RCState nextTState() const throw();

  public:
    const RCCommand& operator=(const std::string& label) throw();
    const RCCommand& operator=(const char* label) throw();
    const RCCommand& operator=(int id) throw();

  };

}

#endif
