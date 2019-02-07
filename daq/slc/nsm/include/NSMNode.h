#ifndef _Belle2_NSMNode_hh
#define _Belle2_NSMNode_hh

#include <daq/slc/base/Connection.h>

#include "daq/slc/nsm/NSMCommand.h"
#include "daq/slc/nsm/NSMState.h"

namespace Belle2 {

  class NSMNode {

  public:
    NSMNode(const std::string& name = "");
    virtual ~NSMNode() {}

  public:
    const std::string& getName() const { return m_name; }
    int getId() const { return m_id; }
    bool isUsed() const { return m_used; }
    NSMState& getState() { return m_state; }
    const NSMState& getState() const { return m_state; }
    void setName(const std::string& name);
    void setUsed(bool used) { m_used = used; }
    void setState(const NSMState& state) { m_state = state; }
    void setId(int id) { m_id = id; }

  protected:
    std::string m_name;
    int m_id;
    bool m_used;
    NSMState m_state;

  };

};

#endif
