#ifndef _Belle2_RunSummary_h
#define _Belle2_RunSummary_h

#include "daq/slc/apps/runcontrold/RunSetting.h"

#include <daq/slc/nsm/NSMNode.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/database/LoggerObject.h>
#include <daq/slc/database/LoggerObjectTable.h>

namespace Belle2 {

  class RunSummary : public LoggerObject {

    typedef std::vector<NSMNode> NSMNodeList;
    typedef std::vector<NSMData> NSMDataList;

  public:
    enum Cause {
      MANUAL = 1, AUTO, RUNEND, ERROR, ERROR_MANUAL
    };

  public:
    RunSummary(const NSMNode& node);
    ~RunSummary() throw();

  public:
    void setState(const NSMNode& node);
    void setCause(Cause cause);
    void setRunSetting(const RunSetting& obj);
    void setNodeState(const NSMNodeList& node_v);
    void setNodeData(const NSMDataList& data_v);

  private:
    const NSMNode& m_node;

  };

}

#endif
