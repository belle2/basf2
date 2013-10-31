#include "DataSender.h"

#include "StringUtil.h"

using namespace Belle2;

const std::string DataSender::getSQLFields() const throw()
{
  return  ", script text, port smallint, event_size int";
}

const std::string DataSender::getSQLLabels() const throw()
{
  return ", script, port, event_size";
}

const std::string DataSender::getSQLValues() const throw()
{
  return  Belle2::form(", '%s', %d, %d", getScript().c_str(),
                       getPort(), getEventSize());
}
