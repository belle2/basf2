#ifndef Belle2_HVChannel_h
#define Belle2_HVChannel_h

namespace Belle2 {

  class HVChannel {

  public:
    HVChannel() {}
    HVChannel(int index, int slot, int channel, bool turnon = false)
      : m_index(index),
        m_slot(slot),
        m_channel(channel),
        m_turnon(turnon) {}
    HVChannel(const HVChannel& config)
      : m_index(config.m_index),
        m_slot(config.m_slot),
        m_channel(config.m_channel),
        m_turnon(config.m_turnon),
        m_rampup(config.m_rampup),
        m_rampdown(config.m_rampdown),
        m_vdemand(config.m_vdemand),
        m_vlimit(config.m_vlimit),
        m_climit(config.m_climit),
        m_vmon(config.m_vmon),
        m_cmon(config.m_cmon),
        m_state(config.m_state) {}
    ~HVChannel() throw() {}

  public:
    int getIndex() const { return m_index; }
    int getSlot() const { return m_slot; }
    int getChannel() const { return m_channel; }
    bool isTurnOn() const { return m_turnon; }
    float getRampUpSpeed() const { return m_rampup; }
    float getRampDownSpeed() const { return m_rampdown; }
    float getVoltageDemand() const { return m_vdemand; }
    float getVoltageLimit() const { return m_vlimit; }
    float getCurrentLimit() const { return m_climit; }
    int getState() const { return m_state; }
    float getVoltageMonitor() const { return m_vmon; }
    float getCurrentMonitor() const { return m_cmon; }
    void setIndex(int id) { m_index = id; }
    void setSlot(int id) { m_slot = id; }
    void setChannel(int id) { m_channel = id; }
    void setTurnOn(bool turnon) { m_turnon = turnon; }
    void setRampUpSpeed(float v) { m_rampup = v; }
    void setRampDownSpeed(float v) { m_rampdown = v; }
    void setVoltageDemand(float v) { m_vdemand = v; }
    void setVoltageLimit(float v) { m_vlimit = v; }
    void setCurrentLimit(float v) { m_climit = v; }
    void setState(int state) { m_state = state; }
    void setVoltageMonitor(float v) { m_vmon = v; }
    void setCurrentMonitor(float v) { m_cmon = v; }

  private:
    int m_index;
    int m_slot;
    int m_channel;
    bool m_turnon;
    float m_rampup;
    float m_rampdown;
    float m_vdemand;
    float m_vlimit;
    float m_climit;
    float m_vmon;
    float m_cmon;
    int m_state;

  };

  typedef std::vector<HVChannel> HVChannelList;

  class HVCrate {

  public:
    HVCrate() {}
    HVCrate(int id, bool used = true)
      : m_id(id), m_used(used) {}
    HVCrate(const HVCrate& config)
      : m_id(config.m_id),
        m_used(config.m_used),
        m_name(config.m_name),
        m_channel(config.m_channel),
        m_port(config.m_port),
        m_host(config.m_host) {}
    ~HVCrate() throw() {}

  public:
    int getId() const { return m_id; }
    bool isUsed() const { return m_used; }
    const std::string& getName() const { return m_name; }
    HVChannelList& getChannels() throw() { return m_channel; }
    const HVChannelList& getChannels() const throw() { return m_channel; }
    void setId(int id) { m_id = id; }
    void setUsed(bool used) { m_used = used; }
    void setName(const std::string& name) { m_name = name; }
    void clearChannel() { m_channel = HVChannelList(); }
    void addChannel(const HVChannel& channel)
    {
      m_channel.push_back(channel);
    }
    const std::string& getHost() const throw() { return m_host; }
    int getPort() const throw() { return m_port; }
    void setHost(const std::string& host) throw() { m_host = host; }
    void setPort(int port) throw() { m_port = port; }

  private:
    int m_id;
    bool m_used;
    std::string m_name;
    HVChannelList m_channel;
    int m_port;
    std::string m_host;

  };

  typedef std::vector<HVCrate> HVCrateList;

}

#endif
