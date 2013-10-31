#ifndef _Belle2_RunConfig_hh
#define _Belle2_RunConfig_hh

#include "NSMData.h"

#include <string>

namespace Belle2 {

  class NSMNode;

  class RunConfig : public NSMData {

  public:
    RunConfig(const std::string& data_name) throw();
    virtual ~RunConfig() throw() {}

  public:
    unsigned int gtNUsed() const throw() { return _nused; }
    bool isUsed(size_t index) const throw() {
      return (index < _nused) ? _used_v[index] : false;
    }
    int getVersion() const throw() { return _version; }
    const std::string& getRunType() const throw() {
      return _run_type;
    }
    const std::string& getOperators() const throw() {
      return _operators;
    }
    int getTriggerMode() const throw() { return _trigger_mode; }
    int getDummyRate() const throw() { return _dummy_rate; }
    int getTriggerLimit() const throw() { return _trigger_limit; }
    void setNUsed(unsigned int nused) throw() { _nused = nused; }
    void setUsed(unsigned int index, bool used) throw() {
      if (index < _nused) _used_v[index] = used;
    }
    void setVersion(unsigned int version) throw() { _version = version; }
    void setRunType(const std::string& run_type) throw() {
      _run_type = run_type;
    }
    void setOperators(const std::string& operators) throw() {
      _operators = operators;
    }
    void setTriggerMode(int mode) throw() { _trigger_mode = mode; }
    void setDummyRate(int rate) throw() { _dummy_rate = rate; }
    void setTriggerLimit(int limit) throw() { _trigger_limit = limit; }
    void read() throw(NSMHandlerException);
    void write() throw(NSMHandlerException);

  private:
    unsigned int _nused;
    bool _used_v[1024];
    std::string _run_type;
    int _version;
    std::string _operators;
    int _trigger_mode;
    int _dummy_rate;
    int _trigger_limit;

  };

}

#endif
