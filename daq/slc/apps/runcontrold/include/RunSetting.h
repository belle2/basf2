#ifndef _Belle2_RunSetting_h
#define _Belle2_RunSetting_h

#include <daq/slc/nsm/NSMNode.h>

#include <daq/slc/database/LoggerObject.h>
#include <daq/slc/database/RunNumberInfo.h>

namespace Belle2 {

  class RunSetting : public LoggerObject {

  public:
    RunSetting(const NSMNode& node);
    ~RunSetting() throw();

  public:
    void setRunNumber(RunNumberInfo info);
    void setOperators(const std::string& operators) {
      setText("operators", operators);
    }
    void setComment(const std::string& comment) {
      setText("comment", comment);
    }
    void setRunControl(const ConfigObject& obj);

  };

}

#endif
