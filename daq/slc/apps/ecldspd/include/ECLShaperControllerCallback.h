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
    virtual void boot(const std::string& opt, const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);

  public:
    std::string col_status(int sh_num) throw(RCHandlerException);

  private:
    static void w_sh_reg_io(const char* ip,
                            int reg_num, int wdata) throw(RCHandlerException);
    static void w_col_reg_io(const char* ip,
                             int reg_num, int wdata) throw(RCHandlerException);

  private:
    bool m_forced;

  private:
    class Worker {
    public:
      Worker(int id, const DBObject& obj)
        : m_id(id), m_obj(obj) {}
      virtual ~Worker() {}
      virtual void run() = 0;
    protected:
      int m_id;
      const DBObject& m_obj;
    };

    class Boot : public Worker {
    public:
      Boot(int id, const DBObject& obj)
        : Worker(id, obj) {}
      virtual ~Boot() {}
      virtual void run();
    };

    class Init : public Worker {
    public:
      Init(int id, const DBObject& obj)
        : Worker(id, obj) {}
      virtual ~Init() {}
      virtual void run();
    };

    class Load : public Worker {
    public:
      Load(int id, const DBObject& obj)
        : Worker(id, obj) {}
      virtual ~Load() {}
      virtual void run();
    };

  };

}

#endif
