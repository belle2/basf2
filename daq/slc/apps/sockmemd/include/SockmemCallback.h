#ifndef _Belle2_SockmemCallback_h
#define _Belle2_SockmemCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/base/ConfigFile.h"

#include <map>

namespace Belle2 {

  class SockmemCallback : public NSMCallback {

    struct tx_t {
      std::string m_host;
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
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  private:
    std::map<std::string, tx_t> m_tx;

  };

}

#endif
