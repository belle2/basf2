#ifndef _Belle2_Node_hh
#define _Belle2_Node_hh

#include <string>
#include <vector>

namespace Belle2 {

  class Node {

  public:
    Node(int version = -1, int id = -1)
      : _version(version), _id(id) {}
    virtual ~Node() throw() {}

  public:
    int getVersion() const { return _version; }
    int getID() const { return _id; }
    void setVersion(int version) { _version = version; }
    void setID(int id) { _id = id; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    int _version;
    int _id;

  };

}

#endif
