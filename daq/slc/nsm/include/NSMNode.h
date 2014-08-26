#ifndef _Belle2_NSMNode_hh
#define _Belle2_NSMNode_hh

#include <daq/slc/base/Connection.h>

#include "daq/slc/nsm/NSMCommand.h"
#include "daq/slc/nsm/NSMState.h"


namespace Belle2 {

  class NSMNode {

  public:
    NSMNode(const std::string& name = "")
      : m_name(name), m_id(-1), m_index(-1), m_error(0) {
      m_excluded = false;
    }
    virtual ~NSMNode() throw() {}

  public:
    const std::string& getName() const throw() { return m_name; }
    int getId() const throw() { return m_id; }
    bool isUsed() const throw() { return m_used; }
    NSMState& getState() throw() { return m_state; }
    const NSMState& getState() const throw() { return m_state; }
    Connection& getConnection() throw() { return m_connection; }
    const Connection& getConnection() const throw() { return m_connection; }
    void setError(int error) throw() { m_error = error; }
    bool isSynchronized() const throw() { return m_sync; }
    int getIndex() const throw() { return m_index; }
    void setName(const std::string& name) throw() { m_name = name; }
    void setUsed(bool used) throw() { m_used = used; }
    void setState(const NSMState& state) throw() { m_state = state; }
    void setId(int id) throw() { m_id = id; }
    void setIndex(int index) throw() { m_index = index; }
    void setSynchronized(bool sync) throw() { m_sync = sync; }
    void setConnection(const Connection& connection) throw() {
      m_connection = connection;
    }
    int getError() const throw() { return m_error; }
    bool isExcluded() const throw() { return m_excluded; }
    void setExcluded(bool excluded) throw() { m_excluded = excluded; }

  protected:
    std::string m_name;
    int m_id;
    int m_index;
    unsigned int m_error;
    bool m_excluded;
    bool m_used;
    bool m_sync;
    NSMState m_state;
    Connection m_connection;

  };

};

#endif
