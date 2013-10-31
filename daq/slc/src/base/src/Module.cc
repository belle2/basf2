#include "Module.h"

#include "Host.h"

#include "StringUtil.h"

using namespace Belle2;

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
         Belle2::form(", %d, '%s'", getProductID(),
                      getLocation().c_str());
}
