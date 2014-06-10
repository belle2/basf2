#ifndef _Belle2_HVCommand_h
#define _Belle2_HVCommand_h

#include <daq/slc/nsm/NSMCommand.h>

namespace Belle2 {

  class HVState;

  class HVCommand : public NSMCommand {

  public:
    static const HVCommand CONFIGURE;
    static const HVCommand STANDBY;
    static const HVCommand SHOULDER;
    static const HVCommand PEAK;
    static const HVCommand RECOVER;
    static const HVCommand TURNON;
    static const HVCommand TURNOFF;
    static const HVCommand HVAPPLY;

  public:
    HVCommand() throw() {}
    HVCommand(const Enum& e) throw() : NSMCommand(e) {}
    HVCommand(const NSMCommand& cmd) throw() : NSMCommand(cmd) {}
    HVCommand(const HVCommand& cmd) throw() : NSMCommand(cmd) {}
    HVCommand(const std::string& label) throw() { *this = label; }
    HVCommand(const char* label) throw() { *this = label; }
    ~HVCommand() throw() {}

  protected:
    HVCommand(int id, const char* label)
    throw() : NSMCommand(id, label) {}

  public:
    //virtual int isAvailable(const State& state) const throw();
    HVState nextState() const throw();

  public:
    const HVCommand& operator=(const std::string& label) throw();
    const HVCommand& operator=(const char* label) throw();

  };

}

#endif


