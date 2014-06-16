#ifndef Belle2_HVChannel_h
#define Belle2_HVChannel_h

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class HVChannel {

  public:
    HVChannel(ConfigObject* obj) : m_obj(obj) {}
    HVChannel(const HVChannel& config)
      : m_obj(config.m_obj) {}
    ~HVChannel() throw() {}

  public:
    void print() throw() { m_obj->print(); }
    int getCrate() const { return m_obj->getInt("crate"); }
    int getSlot() const { return m_obj->getInt("slot"); }
    int getChannel() const { return m_obj->getInt("channel"); }
    bool isTurnOn() const { return m_obj->getBool("turnon"); }
    const ConfigObject& get() const throw() { return *m_obj; }
    void set(ConfigObject* obj) throw() { m_obj = obj; }

    void setCrate(int id) { m_obj->setInt("crate", id); }
    void setSlot(int id) { m_obj->setInt("slot", id); }
    void setChannel(int id) { m_obj->setInt("channel", id); }
    void setTurnOn(bool turnon) { m_obj->setBool("turnon", turnon); }

  private:
    ConfigObject* m_obj;

  };

  typedef std::vector<HVChannel> HVChannelList;

}

#endif
