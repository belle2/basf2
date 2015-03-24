#ifndef _Belle2_RunSetting_h
#define _Belle2_RunSetting_h

#include <daq/slc/nsm/NSMNode.h>

#include <daq/slc/database/DBObject.h>
#include <daq/slc/database/RunNumber.h>

namespace Belle2 {

  class RunSetting : public DBObject {

  public:
    RunSetting(const NSMNode& node);
    ~RunSetting() throw();

  public:
    void setRunNumber(const RunNumber& info);
    void setOperators(const std::string& operators)
    {
      setText("operators", operators);
    }
    void setComment(const std::string& comment)
    {
      setText("comment", comment);
    }
    void setRunControl(const DBObject& obj);

  };

}

#endif
