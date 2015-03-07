#ifndef _Belle2_RCNode_h
#define _Belle2_RCNode_h

#include "daq/slc/nsm/NSMNode.h"

#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCState.h"


namespace Belle2 {

  class RCNode : public NSMNode {

  public:
    RCNode(const std::string& name = "")
      : NSMNode(name), m_sequential(false) {}
    virtual ~RCNode() throw() {}

  public:
    bool isSequential() const throw() { return m_sequential; }
    const std::string& getConfig() const throw() { return m_config; }
    void setSequential(bool seq) throw() { m_sequential = seq; }
    void setConfig(const std::string& config) throw() { m_config = config; }

  private:
    bool m_sequential;
    std::string m_config;

  };

};

#endif
