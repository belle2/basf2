#ifndef _Belle2_HVNode_h
#define _Belle2_HVNode_h

#include "daq/slc/nsm/NSMNode.h"

#include "daq/slc/hvcontrol/HVCommand.h"
#include "daq/slc/hvcontrol/HVState.h"


namespace Belle2 {

  class HVNode : public NSMNode {

  public:
    HVNode(const std::string& name = "") : NSMNode(name) {}
    virtual ~HVNode() throw() {}

  public:
    const std::string& getConfig() const throw() { return m_config; }
    void setConfig(const std::string& config) throw() { m_config = config; }

  private:
    std::string m_config;

  };

};

#endif
