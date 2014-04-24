#include "daq/slc/dqm/MonitorConnection.h"

using namespace Belle2;

const MonitorConnection MonitorConnection::UNKNOWN(-1, "UNKNOWN");
const MonitorConnection MonitorConnection::ON(1, "ON");
const MonitorConnection MonitorConnection::OFF(0, "OFF");

MonitorConnection::MonitorConnection(int value, const std::string& text)
  : _value(value), _text(text) {}

MonitorConnection::~MonitorConnection() throw() {}

MonitorConnection::MonitorConnection()
{
  *this = UNKNOWN;
}

const MonitorConnection& MonitorConnection::operator=(int value) throw()
{
  if (value == ON.getValue()) {
    *this = ON;
  } else if (value == OFF.getValue()) {
    *this = OFF;
  } else {
    *this = UNKNOWN;
  }
  return *this;
}

const MonitorConnection& MonitorConnection::operator=(const MonitorConnection& connection)
throw()
{
  _value = connection.getValue();
  _text = connection.getText();
  return *this;
}

bool MonitorConnection::operator==(int value) const throw()
{
  return _value == value;
}

bool MonitorConnection::operator==(const MonitorConnection& connection) const throw()
{
  return _value == connection._value;
}

namespace Belle2 {

  bool operator==(int value, const MonitorConnection& connection) throw()
  {
    return value == connection.getValue();
  }

  bool operator!=(int value, const MonitorConnection& connection) throw()
  {
    return value != connection.getValue();
  }

};
