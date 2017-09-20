#ifndef _Belle2_TOPFEE_h
#define _Belle2_TOPFEE_h

#include <daq/slc/copper/FEE.h>

#include "daq/slc/copper/top/BoardStackStatus.h"

#include <map>

namespace Belle2 {

  class TOPFEE : public FEE {

  public:
    TOPFEE();
    virtual ~TOPFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void monitor(RCCallback& callback, HSLB& hslb);

  private:
    std::map<int, int> m_numberOfCarriers; /**number of carriers connected to scrods*/
    std::map<int, BoardStackStatus> m_statusMonitor;/**board stack status object for every fee module connected to copper*/
  };

}

#endif
