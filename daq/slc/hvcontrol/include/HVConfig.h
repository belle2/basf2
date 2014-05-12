#ifndef Belle2_HVConfig_h
#define Belle2_HVConfig_h

#include <daq/slc/database/ConfigObject.h>

#include "daq/slc/hvcontrol/HVChannel.h"
#include "daq/slc/hvcontrol/HVValue.h"

namespace Belle2 {

  class HVConfig {

  public:
    HVConfig() throw() { reset(); }
    HVConfig(ConfigObject& obj) throw() { set(obj); }
    HVConfig(const HVConfig& config) throw() { set(config.m_obj); }
    ~HVConfig() throw() {}

  public:
    void print() throw();
    const ConfigObject& get() const throw() { return m_obj; }
    ConfigObject& get() throw() { return m_obj; }
    void reset() throw();
    void set(const ConfigObject& obj) throw();
    void setName(const std::string& name) throw() { m_obj.setName(name); }
    size_t getNChannels() const throw() { return m_channel_v.size(); }
    size_t getNValueSets() const throw() { return m_valueset_v.size(); }
    const HVChannelList& getChannels() const throw() { return m_channel_v; }
    const HVChannel& getChannel(size_t i) const throw() { return m_channel_v[i]; }
    const HVValueSetList& getValueSets() const throw() { return m_valueset_v; }
    const HVValueSet& getValueSet(size_t i) const throw() { return m_valueset_v[i]; }
    const DBObject& getExtra() const throw() { return m_obj.getObject("extra"); }
    HVChannelList& getChannels() throw() { return m_channel_v; }
    HVChannel& getChannel(size_t i) throw() { return m_channel_v[i]; }
    HVValueSetList& getValueSets() throw() { return m_valueset_v; }
    HVValueSet& getValueSet(size_t i) throw() { return m_valueset_v[i]; }
    DBObject& getExtra() throw() { return m_obj.getObject("extra"); }

  private:
    ConfigObject m_obj;
    HVChannelList m_channel_v;
    HVValueSetList m_valueset_v;

  };

}

#endif
