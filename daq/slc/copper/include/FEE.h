#ifndef _Belle2_FEE_h
#define _Belle2_FEE_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/database/DBObject.h>

#include <daq/slc/copper/HSLB.h>

namespace Belle2 {

  class FEE {

  public:
    FEE() {}
    virtual ~FEE() throw() {}

  public:
    virtual void init(RCCallback& /*callback*/, HSLB& /*hslb*/) {}
    virtual void boot(HSLB& hslb, const DBObject& obj) = 0;
    virtual void load(HSLB& hslb, const DBObject& obj) = 0;
    virtual void monitor(RCCallback& /*callback*/, HSLB& /*hslb*/) {}

  public:
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }

  private:
    std::string m_name;

  };

}

#endif
