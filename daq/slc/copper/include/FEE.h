#ifndef _Belle2_FEE_h
#define _Belle2_FEE_h

#include <daq/slc/copper/FEEConfig.h>
#include <daq/slc/copper/HSLB.h>

namespace Belle2 {

  class FEE {

  public:
    FEE() {}
    virtual ~FEE() throw() {}

  public:
    virtual bool boot(HSLB& hslb, const FEEConfig& conf);
    virtual bool load(HSLB& hslb, const FEEConfig& conf);

  public:
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }

  private:
    std::string m_name;

  };

}

#endif
