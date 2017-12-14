#ifndef _Belle2_ERRORNo_hh
#define _Belle2_ERRORNo_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class ERRORNo : public Enum {

  public:
    static const ERRORNo UNKNOWN;
    static const ERRORNo NSMONLINE;
    static const ERRORNo NSMSENDREQ;
    static const ERRORNo NSMMEMOPEN;
    static const ERRORNo NSMINIT;
    static const ERRORNo DATABASE;

  public:
    ERRORNo() throw() {}
    ERRORNo(const Enum& e) throw() : Enum(e) {}
    ERRORNo(const ERRORNo& cmd) throw() : Enum(cmd) {}
    ERRORNo(const char* label) throw() { *this = label; }
    ERRORNo(int id) throw() { *this = id; }
    ~ERRORNo() throw() {}

  protected:
    ERRORNo(int id, const char* label)
    throw() : Enum(id, label) {}

  public:
    const ERRORNo& operator=(const std::string& label) throw();
    const ERRORNo& operator=(const char* label) throw();
    const ERRORNo& operator=(int id) throw();

  };

}

#endif
