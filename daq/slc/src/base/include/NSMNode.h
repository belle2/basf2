#ifndef _Belle2_NSMNode_hh
#define _Belle2_NSMNode_hh

#include "base/Command.h"
#include "base/Node.h"
#include "base/State.h"
#include "base/Connection.h"
#include "base/DataObject.h"

#include <vector>

namespace Belle2 {

  class NSMNode : public Node {

  public:
    NSMNode(const std::string& name = "", DataObject* obj = NULL)
      : _obj(obj), _name(name) { _node_id = _index = -1; }
    virtual ~NSMNode() throw() {}

  public:
    DataObject* getData() { return _obj; }
    const std::string& getName() const throw() { return _name; }
    const std::string& getType() const throw() { return _type; }
    bool isUsed() const throw() { return _used; }
    State& getState() throw() { return _state; }
    const State& getState() const throw() { return _state; }
    Connection& getConnection() throw() { return _connection; }
    const Connection& getConnection() const throw() { return _connection; }
    int getNodeID() const throw() { return _node_id; }
    bool isSynchronize() const throw() { return _sync; }
    int getIndex() const throw() { return _index; }
    void setName(const std::string& name) throw() { _name = name; }
    void setUsed(bool used) throw() { _used = used; }
    void setState(const State& state) throw() { _state = state; }
    void setConnection(const Connection& connection) throw() {
      _connection = connection;
    }
    void setNodeID(int id) throw() { _node_id = id; }
    void setIndex(int index) throw() { _index = index; }
    void setSynchronize(bool sync) throw() { _sync = sync; }

  private:
    std::string _type;

  protected:
    DataObject* _obj;
    std::string _name;
    bool _used;
    State _state;
    Connection _connection;
    int _node_id;
    int _index;
    bool _sync;

  };

};

#endif
