#ifndef _Belle2_NSMCommand_hh
#define _Belle2_NSMCommand_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class NSMCommand : public Enum {

  public:
    static const NSMCommand OK;
    static const NSMCommand STATE;
    static const NSMCommand ERROR;
    static const NSMCommand FATAL;
    static const NSMCommand LOG;
    static const NSMCommand NSMGET;
    static const NSMCommand NSMSET;
    static const NSMCommand DBGET;
    static const NSMCommand DBSET;
    static const NSMCommand LISTGET;
    static const NSMCommand LISTSET;
    static const NSMCommand EXCLUDE;
    static const NSMCommand INCLUDE;

  public:
    static const int SUGGESTED = 2;
    static const int ENABLED = 1;
    static const int DISABLED = 0;

  public:
    NSMCommand() throw() {}
    NSMCommand(const Enum& e) throw() : Enum(e) {}
    NSMCommand(const NSMCommand& cmd) throw() : Enum(cmd) {}
    NSMCommand(const char* label) throw() { *this = label; }
    NSMCommand(const std::string& label) throw() { *this = label; }
    NSMCommand(int id) throw() { *this = id; }
    ~NSMCommand() throw() {}

  protected:
    NSMCommand(int id, const char* label)
    throw() : Enum(id, label) {}

  public:
    const NSMCommand& operator=(const std::string& label) throw();
    const NSMCommand& operator=(const char* label) throw();
    const NSMCommand& operator=(int id) throw();

  };

}

#endif
