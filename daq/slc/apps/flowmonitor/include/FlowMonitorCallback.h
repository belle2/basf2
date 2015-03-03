#ifndef _Belle2_FlowMonitor_h
#define _Belle2_FlowMonitor_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class FlowMonitorCallback : public RCCallback {

    typedef std::vector<NSMNode> NSMNodeList;

  public:
    FlowMonitorCallback(const std::string& conf);
    virtual ~FlowMonitorCallback() throw() {}

  public:
    virtual void timeout(NSMCommunicator& com) throw();

  private:
    NSMNode m_runcontrol;
    NSMNodeList m_node;
    NSMDataList m_data;

  };

}

#endif
