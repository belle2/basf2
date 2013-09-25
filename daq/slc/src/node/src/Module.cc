#include "Module.hh"

#include "Host.hh"

#include <util/StringUtil.hh>

using namespace B2DAQ;

const std::string Module::getSQLFields() const throw()
{
  return Node::getSQLFields() + ", product_id int, location text";
}

const std::string Module::getSQLLabels() const throw()
{
  return Node::getSQLLabels() + ", product_id, location";
}

const std::string Module::getSQLValues() const throw()
{
  return Node::getSQLValues() +
         B2DAQ::form(", %d, '%s'", getProductID(),
                     getLocation().c_str());
}
