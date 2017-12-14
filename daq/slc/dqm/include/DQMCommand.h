#ifndef _Belle2_DQMCommand_h
#define _Belle2_DQMCommand_h

#include <daq/slc/nsm/NSMCommand.h>

namespace Belle2 {

  class DQMCommand : public NSMCommand {

  public:
    static const DQMCommand DQMDIRGET;
    static const DQMCommand DQMDIRSET;
    static const DQMCommand DQMHISTGET;
    static const DQMCommand DQMHISTSET;
    static const DQMCommand DQMLISTGET;
    static const DQMCommand DQMLISTSET;
    static const DQMCommand DQMFILEGET;
    static const DQMCommand DQMFILESET;
    static const DQMCommand DQMRESET;

  public:
    DQMCommand() throw() {}
    DQMCommand(const Enum& e) throw() : NSMCommand(e) {}
    DQMCommand(const NSMCommand& e) throw() : NSMCommand(e) {}
    DQMCommand(const DQMCommand& cmd) throw() : NSMCommand(cmd) {}
    DQMCommand(const std::string& label) throw() { *this = label; }
    DQMCommand(const char* label) throw() { *this = label; }
    DQMCommand(int id) throw() { *this = id; }
    ~DQMCommand() throw() {}

  protected:
    DQMCommand(int id, const char* label)
    throw() : NSMCommand(id, label) {}

  public:
    const DQMCommand& operator=(const std::string& label) throw();
    const DQMCommand& operator=(const char* label) throw();
    const DQMCommand& operator=(int id) throw();

  };

}

#endif
