#ifndef _B2DAQ_FTSW_hh
#define _B2DAQ_FTSW_hh

#include "Module.hh"

#include <string>
#include <vector>

namespace B2DAQ {

  class FEEModule;

  class FTSW : public Module {

  public:
    static const size_t MAX_MODULES = 16;

  public:
    static const int TRIG_NORMAL;
    static const int TRIG_IN;
    static const int TRIG_TLU;
    static const int TRIG_PULSE;
    static const int TRIG_REVO;
    static const int TRIG_RANDOM;
    static const int TRIG_POSSION;
    static const int TRIG_ONCE;
    static const int TRIG_STOP;

  public:
    typedef std::vector<FEEModule*> FEEModuleList;

  public:
    FTSW(int channel = -1)
      : _channel(channel), _firmware(), _used(true),
	_module_v(MAX_MODULES), _module_i(0) {}
    virtual ~FTSW() throw() {}

  public:
    int getChannel() const throw() { return _channel; }
    const std::string& getFirmware() const { return _firmware; }
    bool isUsed() const { return _used; }
    FEEModuleList& getFEEModules() throw() { return _module_v; }
    const FEEModuleList& getFEEModules() const throw() { return _module_v; }
    void setChannel(int channel) throw() { _channel = channel; }
    void setFirmware(const std::string& firmware) { _firmware = firmware; }
    void setUsed(bool used) { _used = used; }
    void clearModules() throw() { _module_i = 0; }
    void addFEEModule(FEEModule* module) throw() {
      if ( _module_i < MAX_MODULES ) {
	_module_v[_module_i] = module;
	_module_i++;
      }
    }
    int getTriggerMode() const throw() { return _trigger_mode; }
    void setTriggerMode(int mode) throw() { _trigger_mode = mode; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    int _channel;
    std::string _firmware;
    bool _used;
    FEEModuleList _module_v;
    size_t _module_i;
    int _trigger_mode;
    
  };

}

#endif
