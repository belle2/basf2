#ifndef _Belle2_TTCtrlCallback_hh
#define _Belle2_TTCtrlCallback_hh

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/runcontrol/RCNode.h"

#include <daq/slc/nsm/NSMData.h>

#include <map>

#include <ftprogs2/ftsw.h>

namespace Belle2 {

  class TTCtrlCallback : public RCCallback {

  public:
    TTCtrlCallback(int ftswid, const std::string& ttdname);
    virtual ~TTCtrlCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void boot(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject&) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void ok(const char* nodename, const char* data) throw();
    virtual void error(const char* nodename, const char* data) throw();
    void trigft() throw(RCHandlerException);
    void resetft() throw();
    void trigio(const std::string& type) throw(RCHandlerException);

  private:
    int m_ftswid;
    RCNode m_ttdnode;
    std::map<std::string, int> m_trgcommands;

  private:
    void send(const NSMMessage& msg) throw(RCHandlerException);
    void statftx(ftsw_t* ftsw, int ftswid);

  };

}

#endif
