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
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);

  public:
    std::string col_status(int sh_num) throw(RCHandlerException);

  private:
    void sh_boot(int shm_num) throw(RCHandlerException);
    void sh_init_ecldsp(int sh_num, int adr) throw(IOException);
    void sh_reg_io_write(const std::string& hostname, int shm_num,
                         int adr, int val) throw(RCHandlerException);
    void col_reg_io_write(const std::string& hostname,
                          int adr, int val)throw(RCHandlerException);

  private:
    bool m_forced;

  };

}

#endif
