#include "Host.h"

#include "StringUtil.h"

using namespace Belle2;

const std::string Host::getSQLFields() const throw()
{
  return Module::getSQLFields() +
         ", hostname text, type text";
  //", hostname text, mac_address text, hardware text, boot_server text, boot_image text";
}

const std::string Host::getSQLLabels() const throw()
{
  return Module::getSQLLabels() +
         ", hostname, type";
  //  ", hostname, mac_address, hardware, boot_server, boot_image";
}

const std::string Host::getSQLValues() const throw()
{
  return Module::getSQLValues() +
         Belle2::form(", '%s', '%s'", getName().c_str(), getType().c_str());
  /*
    Belle2::form(", '%s', '%s', '%s', '%s', '%s'",
    getName().c_str(), getMacAddress().c_str(),
    getHardware().c_str(), getBootServer().c_str(),
    getBootImage().c_str());
  */
}
