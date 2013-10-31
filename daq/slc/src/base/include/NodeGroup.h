#ifndef _Belle2_NodeGroup_hh
#define _Belle2_NodeGroup_hh

#include "NSMNode.h"

#include <vector>

namespace Belle2 {

  class NodeGroup {

  public:
    NodeGroup() {}
    virtual ~NodeGroup() throw() {}

  public:
    const std::string& getName() const throw() { return _name; }
    int getRows() const throw() { return _rows; }
    int getCols() const throw() { return _cols; }
    std::vector<NSMNode*>& getNodes() throw() { return _node_v; }
    void setName(const std::string& name) throw() { _name = name; }
    void setRows(int rows) throw() { _rows = rows; }
    void setCols(int cols) throw() { _cols = cols; }
    void addNode(NSMNode* node) throw() { _node_v.push_back(node); }

  private:
    std::string _name;
    size_t _rows, _cols;
    std::vector<NSMNode*> _node_v;

  };

};

#endif
