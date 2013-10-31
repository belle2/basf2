#ifndef _Belle2_NSMNode_hh
#define _Belle2_NSMNode_hh

#include "Command.h"
#include "Node.h"
#include "State.h"
#include "Connection.h"

#include <vector>

namespace Belle2 {

  class Host;

  class NSMNode : public Node {

  public:
    NSMNode(const std::string& name = "",
            const std::string& type = "nsmnode")
      : _type(type), _name(name) { _host = NULL; _node_id = _index = -1; }
    virtual ~NSMNode() throw() {}

  public:
    const std::string& getName() const throw() { return _name; }
    const std::string& getType() const throw() { return _type; }
    bool isUsed() const throw() { return _used; }
    const Host* getHost() const throw() { return _host; }
    Host* getHost() throw() { return _host; }
    State& getState() throw() { return _state; }
    const State& getState() const throw() { return _state; }
    Connection& getConnection() throw() { return _connection; }
    const Connection& getConnection() const throw() { return _connection; }
    int getNodeID() const throw() { return _node_id; }
    bool isSynchronize() const throw() { return _sync; }
    int getIndex() const throw() { return _index; }
    void setName(const std::string& name) throw() { _name = name; }
    void setUsed(bool used) throw() { _used = used; }
    void setHost(Host* host) throw() { _host = host; }
    void setState(const State& state) throw() { _state = state; }
    void setConnection(const Connection& connection) throw() {
      _connection = connection;
    }
    void setNodeID(int id) throw() { _node_id = id; }
    void setIndex(int index) throw() { _index = index; }
    void setSynchronize(bool sync) throw() { _sync = sync; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();
    virtual int revision() const throw() { return 1; }
    virtual const std::string format() const throw() { return ""; }
    virtual int getParams(const Command& command, unsigned int* pars,
                          std::string& datap) { return 0; }
    virtual void setParams(const Command& command, int npar,
                           const unsigned int* pars, const std::string& datap) {}

  private:
    std::string _type;

  protected:
    std::string _name;
    bool _used;
    Host* _host;
    State _state;
    Connection _connection;
    int _node_id;
    int _index;
    bool _sync;

  };

};

#endif
