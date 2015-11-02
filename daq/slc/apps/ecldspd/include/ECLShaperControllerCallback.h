#ifndef _Belle2_ECLShaperControllerCallback_h
#define _Belle2_ECLShaperControllerCallback_h

#include "daq/slc/runcontrol/RCCallback.h"

namespace Belle2 {

  class ECLShaperControllerCallback : public RCCallback {

  public:
    ECLShaperControllerCallback() : m_forced(true) {}
    virtual ~ECLShaperControllerCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void boot(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);

  public:
    std::string col_status(int sh_num) throw(RCHandlerException);

  private:
    void w_sh_reg_io(const char* ip,
                     int reg_num, int wdata) throw(RCHandlerException);
    void w_col_reg_io(const char* ip,
                      int reg_num, int wdata) throw(RCHandlerException);

  private:
    bool m_forced;

  };

}

#endif
