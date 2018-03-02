#ifndef _Belle2_SockmemCallback_h
#define _Belle2_SockmemCallback_h

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/base/ConfigFile.h"

#include <map>

namespace Belle2 {

  class SockmemCallback : public RCCallback {

    struct tx_t {
      std::string m_host;
      std::string m_type;
      std::string m_local;
      std::string m_remote;
      double m_ntxq;
      double m_nrxq;
      double m_bytes;
      double m_rate;
      bool m_active;

    public:
      void update(SockmemCallback& callback, std::string state,
                  double nrxq, double ntxq, double bytes,
                  std::string local, std::string remote,
                  double t, double t0);
    };

  public:
    static StringList popen(const std::string& cmd);

  public:
    SockmemCallback(const std::string& nodename, int timout, ConfigFile& conf);
    virtual ~SockmemCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);

  private:
    std::map<std::string, tx_t> m_tx;
    NSMNode m_rcnode;
    bool m_aborted;

  };

}

#endif
