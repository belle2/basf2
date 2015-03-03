#ifndef _Belle2_RCNode_h
#define _Belle2_RCNode_h

#include "daq/slc/nsm/NSMNode.h"

#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCState.h"


namespace Belle2 {

  class RCNode : public NSMNode {

  public:
    RCNode(const std::string& name = "")
      : NSMNode(name), m_excluded(false),
        m_sequential(false) {}
    virtual ~RCNode() throw() {}

  public:
    bool isExcluded() const throw() { return m_excluded; }
    bool isSequential() const throw() { return m_sequential; }
    const std::string& getConfig() const throw() { return m_config; }
    void setExcluded(bool excluded) throw() { m_excluded = excluded; }
    void setSequential(bool seq) throw() { m_sequential = seq; }
    void setConfig(const std::string& config) throw() { m_config = config; }

  private:
    bool m_excluded;
    bool m_sequential;
    std::string m_config;
    NSMState m_state;

  };

};

#endif
