#ifndef Belle2_HVConfig_h
#define Belle2_HVConfig_h

#include <daq/slc/database/DBObject.h>

#include "daq/slc/hvcontrol/HVChannel.h"
#include "daq/slc/hvcontrol/HVValue.h"

namespace Belle2 {

  class HVConfig {

  public:
    HVConfig() throw() { reset(); }
    HVConfig(DBObject& obj) throw() { set(obj); }
    HVConfig(const HVConfig& config) throw()
    {
      set(config.m_obj);
      m_crate_v = config.m_crate_v;
    }
    ~HVConfig() throw() {}

  public:
    const std::string& getName() const throw() { return m_obj.getName(); }
    const HVCrateList& getCrates() const throw() { return m_crate_v; }
    HVCrateList& getCrates() throw() { return m_crate_v; }
    HVChannel& getChannel(int crate, int slot, int channel);
    HVChannel& getChannel(int crate, int index);

  public:
    void set(const DBObject& obj) throw();
    const DBObject& get() const throw() { return m_obj; }
    DBObject& get() throw() { return m_obj; }
    void reset() throw();

  private:
    DBObject m_obj;
    HVCrateList m_crate_v;

  };

  typedef std::vector<HVConfig> HVConfigList;

}

#endif
