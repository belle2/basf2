#ifndef _Belle2_FlowMonitor_h
#define _Belle2_FlowMonitor_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/base/StringUtil.h>

namespace Belle2 {

  class FlowMonitorCallback : public NSMCallback {

    typedef std::vector<NSMData> NSMDataList;
    typedef std::map<std::string, NSMNode> NSMNodeMap;

  public:
    FlowMonitorCallback(const std::string& node, const std::string& rcnode);
    virtual ~FlowMonitorCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void nsmdataset(NSMCommunicator& com, NSMData& data) throw();

  public:
    const StringList& getDataNames() const throw() { return m_datanames; }
    const std::string& getDataName(int index) const throw() { return m_datanames[index]; }
    void setNeventsThreshold(unsigned int n) { m_nevents_th = n; }

  private:
    NSMNode m_runcontrol;
    StringList m_names;
    StringList m_hostnames;
    StringList m_datanames;
    NSMNodeMap m_nodes;
    unsigned int m_nevents_th;
    RCState m_state;

  };

}

#endif
