#ifndef _Belle2_NSMState_hh
#define _Belle2_NSMState_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class NSMState : public Enum {

  public:
    static const NSMState ONLINE_S;

  public:
    NSMState() : Enum(Enum::UNKNOWN) {}
    NSMState(const Enum& num) : Enum(num) {}
    NSMState(const NSMState& st) : Enum(st) {}
    ~NSMState()  {}

  protected:
    NSMState(int id, const char* label)
      : Enum(id, label) {}

  };

}

#endif
