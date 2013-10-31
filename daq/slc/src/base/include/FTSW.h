#ifndef _Belle2_FTSW_hh
#define _Belle2_FTSW_hh

#include "Module.h"

#include <string>
#include <vector>

namespace Belle2 {

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
    FTSW(int channel = -1)
      : _channel(channel), _firmware(), _used(true),
        _trigger_mode(TRIG_NORMAL), _dummy_rate(0),
        _trigger_limit(-1) {}
    virtual ~FTSW() throw() {}

  public:
    int getChannel() const throw() { return _channel; }
    const std::string& getFirmware() const { return _firmware; }
    bool isUsed() const { return _used; }
    void setChannel(int channel) throw() { _channel = channel; }
    void setFirmware(const std::string& firmware) { _firmware = firmware; }
    void setUsed(bool used) { _used = used; }
    unsigned int getTriggerMode() const throw() { return _trigger_mode; }
    unsigned int getTriggerLimit() const throw() { return _trigger_limit; }
    unsigned int getDummyRate() const throw() { return _dummy_rate; }
    void setTriggerMode(unsigned int mode) throw() { _trigger_mode = mode; }
    void setTriggerLimit(unsigned int limit) throw() { _trigger_limit = limit; }
    void setDummyRate(unsigned int rate) throw() { _dummy_rate = rate; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    int _channel;
    std::string _firmware;
    bool _used;
    int _trigger_mode;
    int _dummy_rate;
    int _trigger_limit;

  };

}

#endif
