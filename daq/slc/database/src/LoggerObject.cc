#include "daq/slc/database/LoggerObject.h"

using namespace Belle2;

LoggerObject::LoggerObject()
{
  setConfig(false);
}

LoggerObject::LoggerObject(const LoggerObject& obj)
  : ConfigObject(obj)
{
  setConfig(false);
}

LoggerObject::~LoggerObject() throw()
{
  reset();
}


