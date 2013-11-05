#ifndef _Belle2_RunConfig_hh
#define _Belle2_RunConfig_hh

#include "base/DataObject.h"

namespace Belle2 {

  class RunConfig : public DataObject {

  public:
    RunConfig(const std::string& data_name, int revision) throw();
    virtual ~RunConfig() throw() {}

  public:
    int getRunNumber() const throw() { return getIntValue("run_number"); }
    int getExpNumber() const throw() { return getIntValue("exp_number"); }
    int getVersion() const throw() { return getIntValue("version"); }
    int getStartTime() const throw() { return getIntValue("start_time"); }
    const std::string& getRunType() const throw() { return getTextValue("run_type"); }
    const std::string& getOperators() const throw() {return getTextValue("operators"); }
    const std::string& getDescription() const throw() { return _description; }
    void setRunNumber(int run_number) throw() { setIntValue("run_number", run_number); }
    void setExpNumber(int exp_number) throw() { setIntValue("exp_number", exp_number); }
    void setVersion(int version) throw() { setIntValue("version", version); }
    void setStartTime(int start_time) throw() { setIntValue("start_time", start_time); }
    void setRunType(const std::string& run_type) throw() { setTextValue("run_type", run_type); }
    void setOperators(const std::string& operators) throw() { setTextValue("operators", operators); }
    void setDescription(const std::string& description) throw() { _description = description; }

  private:
    std::string _description;

  };

}

#endif
