#ifndef _Belle2_FEEModule_hh
#define _Belle2_FEEModule_hh

#include "Module.h"

#include <map>

namespace Belle2 {

  class FEEModule : public Module {

  public:
    class Register  {

    public:
      Register() { _size = 1; }
      ~Register() throw() {}

    public:
      const std::string& getName() const { return _name; }
      int getAddress() const { return _address; }
      int getValue(int ch) const { return _value_v[ch]; }
      const std::vector<int>& getValues() const { return _value_v; }
      size_t length() const { return _value_v.size(); }
      size_t getSize() const { return _size; }
      void setName(const std::string& name) { _name = name; }
      void setAddress(int address) { _address = address; }
      void setValues(const std::vector<int>& value_v) { _value_v = value_v; }
      void setValues(int value) {
        for (size_t ch = 0; ch < _value_v.size(); ch++)
          _value_v[ch] = value;
      }
      void setValue(int ch, int value) { _value_v[ch] = value; }
      void setLength(size_t length) { _value_v.resize((length > 0) ? length : 1); }
      void setSize(size_t size) { if (size > 0) _size = size; }
      void set(const std::string& name, int address, size_t length, size_t size) {
        _name = name; _address = address; setLength(length); setSize(size);
      }
      size_t count() const { return sizeof(int) * length(); }

    private:
      std::string _name;
      int _address;
      std::vector<int> _value_v;
      size_t _size;

    };

  public:
    typedef std::vector<Register> RegisterList;

  public:
    FEEModule(const std::string& type = "")
      : _type(type) {}
    virtual ~FEEModule() throw() {}

  public:
    const std::string& getType() const throw() { return _type; }
    const std::vector<Register>& getRegisters() const throw() { return _reg_v; }
    std::vector<Register>& getRegisters() throw() { return _reg_v; }
    Register& getRegister(int index) throw() { return _reg_v[index]; }
    Register* getRegister(const std::string& name) throw();
    void setType(const std::string& type) throw() { _type = type; }
    void addRegister(const Register& reg) throw();
    void setRegister(int index, const Register& reg) throw() { _reg_v[index] = reg; }
    void setRegisters(const RegisterList& reg_v) throw() { _reg_v = reg_v; }
    void clearRegisters() throw() { _reg_v = RegisterList(); }
    size_t count() const throw();
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    std::string _type;
    RegisterList _reg_v;

  };

}

#endif
