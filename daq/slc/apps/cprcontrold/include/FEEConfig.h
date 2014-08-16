#ifndef _Belle2_FEEConfig_h
#define _Belle2_FEEConfig_h

#include <daq/slc/database/ConfigObject.h>

#include <vector>
#include <string>

namespace Belle2 {

  class FEEConfig {

  public:
    class Register {

    public:
      Register() {}
      Register(const std::string& name,
               int address, int size)
        : m_name(name), m_address(address), m_size(size) {}
      ~Register() throw() {}

    public:
      void setParamName(const std::string& name) { m_name = name; }
      void setAddress(int address) { m_address = address; }
      void setSize(int size) { m_size = size; }
      const std::string& getParamName() const { return m_name; }
      int getAddress() const { return m_address; }
      int getSize() const { return m_size; }
      void read(const ConfigObject& obj);

    private:
      std::string m_name;
      int m_address;
      int m_size;

    };

    class Parameter {

    public:
      Parameter() {}
      Parameter(const std::string& name,
                int index, int value)
        : m_name(name), m_index(index), m_value(value) {}
      ~Parameter() throw() {}

    public:
      void setName(const std::string& name) { m_name = name; }
      void setIndex(int index) { m_index = index; }
      void setValue(int value) { m_value = value; }
      const std::string& getName() const { return m_name; }
      int getIndex() const { return m_index; }
      int getValue() const { return m_value; }
      void read(const ConfigObject& obj);

    private:
      std::string m_name;
      int m_index;
      int m_value;

    };

    typedef std::vector<Register> RegisterList;
    typedef std::vector<Parameter> ParameterList;

  public:
    FEEConfig() {}
    ~FEEConfig() {}

  public:
    void setSlot(int slot) { m_slot = slot; }
    void setBoard(const std::string& board) { m_board = board; }
    void setFirmware(const std::string& firmware) { m_firmware = firmware; }
    void clear() {
      m_slot = -1;
      m_board = m_firmware = "";
      m_register = RegisterList();
      m_parameter = ParameterList();
    }
    int getSlot() const { return m_slot; }
    const std::string& getBoard() const { return m_board; }
    const std::string& getFirmware() const { return m_firmware; }
    RegisterList& getRegisters() { return m_register; }
    ParameterList& getParameters() { return m_parameter; }
    void read(const ConfigObject& obj);

  private:
    int m_slot;
    std::string m_board;
    std::string m_firmware;
    RegisterList m_register;
    ParameterList m_parameter;

  };

}

#endif
