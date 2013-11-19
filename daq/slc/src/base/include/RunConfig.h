#ifndef _Belle2_RunConfig_hh
#define _Belle2_RunConfig_hh

#include "base/DataObject.h"

namespace Belle2 {

  class RunConfig : public DataObject {

  public:
    RunConfig(const std::string& data_name, int revision) throw();
    virtual ~RunConfig() throw() {}

  public:
    int getRunNumber() const throw() { return getInt("run_number"); }
    int getExpNumber() const throw() { return getInt("exp_number"); }
    int getVersion() const throw() { return getInt("version"); }
    int getStartTime() const throw() { return getInt("start_time"); }
    const std::string getRunType() const throw() { return getText("run_type"); }
    const std::string getOperators() const throw() {return getText("operators"); }
    const std::string getDescription() const throw() { return _description; }
    void setRunNumber(int run_number) throw() { setInt("run_number", run_number); }
    void setExpNumber(int exp_number) throw() { setInt("exp_number", exp_number); }
    void setVersion(int version) throw() { setInt("version", version); }
    void setStartTime(int start_time) throw() { setInt("start_time", start_time); }
    void setRunType(const std::string& run_type) throw() { setText("run_type", run_type); }
    void setOperators(const std::string& operators) throw() { setText("operators", operators); }
    void setDescription(const std::string& description) throw() { _description = description; }

  private:
    std::string _description;

  };

}

#endif
