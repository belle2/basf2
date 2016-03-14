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
    static const NSMCommand LOGSET;
    static const NSMCommand LOGGET;
    static const NSMCommand VLISTGET;
    static const NSMCommand VLISTSET;
    static const NSMCommand VGET;
    static const NSMCommand VSET;
    static const NSMCommand VREPLY;

  public:
    static const NSMCommand DATAGET;
    static const NSMCommand DATASET;
    static const NSMCommand DBGET;
    static const NSMCommand DBSET;
    static const NSMCommand DBLISTGET;
    static const NSMCommand DBLISTSET;
    static const NSMCommand LOGLIST;

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

  public:
    NSMCommand(int id, const char* label) throw()
      : Enum(id, label) {}

  public:
    const NSMCommand& operator=(const std::string& label) throw();
    const NSMCommand& operator=(const char* label) throw();
    const NSMCommand& operator=(int id) throw();

  };

}

#endif
