#ifndef _B2DAQ_Module_hh
#define _B2DAQ_Module_hh

#include "Node.hh"
#include "State.hh"
#include "Connection.hh"

#include <vector>

namespace B2DAQ {

  class Host;

  class Module : public Node {

  public:
    Module() : _product_id(-1) {}
    virtual ~Module() throw() {}

  public:
    int getProductID() const throw() { return _product_id; }
    void setProductID(int id) throw() { _product_id = id; }
    const std::string& getLocation() const throw() {
      return _location;
    }
    void setLocation(const std::string& location) throw() {
      _location = location;
    }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    int _product_id;
    std::string _location;
    
  };

};

#endif
