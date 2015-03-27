#ifndef _Belle2_FEEConfig_h
#define _Belle2_FEEConfig_h

#include <vector>
#include <string>

namespace Belle2 {

  class DBObject;

  class FEEConfig {

  public:
    struct Reg {
      std::string name;
      int adr;
      int size;
      int val;
    };

    typedef std::vector<Reg> RegList;

  public:
    FEEConfig() {}
    FEEConfig(const DBObject& obj);
    ~FEEConfig() {}

  public:
    void setFirmware(const std::string& firmware) { m_firmware = firmware; }
    const char* getFirmware() const { return m_firmware.c_str(); }
    void setStream(const std::string& stream) { m_stream = stream; }
    const char* getStream() const { return m_stream.c_str(); }
    RegList& getRegList() { return m_regs; }
    const RegList& getRegList() const { return m_regs; }
    bool read(const DBObject& obj);

  private:
    std::string m_firmware;
    std::string m_stream;
    RegList m_regs;

  };

}

#endif
