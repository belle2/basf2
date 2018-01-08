#ifndef _Belle2_FEE_h
#define _Belle2_FEE_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/database/DBObject.h>

#include <daq/slc/copper/HSLB.h>

namespace Belle2 {

  class FEE {

  public:
    FEE(const std::string& name): m_name(name) {}
    virtual ~FEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& /*hslb*/, const DBObject& /*obj*/) {}
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj) = 0;
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj) = 0;
    virtual void start(RCCallback& callback, HSLB& hslb) {}
    virtual void stop(RCCallback& callback, HSLB& hslb) {}
    virtual void monitor(RCCallback& /*callback*/, HSLB& /*hslb*/) {}
    virtual void readback(RCCallback& callback, HSLB& hslb, const DBObject& obj) {};

  public:
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }
    unsigned int mask(unsigned int val, int max, int min);
    unsigned int mask(int max, int min);

  private:
    std::string m_name;

  };

}

#endif
