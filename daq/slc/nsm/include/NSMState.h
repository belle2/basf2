#ifndef _Belle2_NSMState_hh
#define _Belle2_NSMState_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class NSMState : public Enum {

  public:
    NSMState() throw() : Enum(Enum::UNKNOWN) {}
    NSMState(const Enum& num) throw() : Enum(num) {}
    NSMState(const NSMState& st) throw() : Enum(st) {}
    ~NSMState()  throw() {}

  protected:
    NSMState(int id, const char* label)
    throw() : Enum(id, label) {}

  public:
    inline const NSMState& operator=(const NSMState& e) throw() {
      *this = Enum::operator=(e);
      return *this;
    }

  };

}

#endif
