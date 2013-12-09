#ifndef _Belle2_RunConfig_hh
#define _Belle2_RunConfig_hh

#include <daq/slc/base/DataObject.h>
#include <daq/slc/base/NSMNode.h>

namespace Belle2 {

  class RCMaster;

  class RunConfig : public DataObject {

  public:
    RunConfig(const std::string& data_name = "",
              int revision = -1) throw();
    virtual ~RunConfig() throw() {}

  public:
    const std::string getRunType() const throw() { return getText("run_type"); }
    const std::string getDescription() const throw() { return getText("description"); }
    void setRunType(const std::string& run_type) throw() { setText("run_type", run_type); }
    void setDescription(const std::string& description) throw() {
      setText("_description", description);
    }
    void add(std::map<std::string, DataObject*>& data_m,
             std::vector<NSMNode*>& node_v);
    void update();
    void setMaster(RCMaster* master) { _master = master; }

  private:
    std::map<std::string, DataObject*> _data_m;
    RCMaster* _master;

  };

}

#endif
