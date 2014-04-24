#include "VMEControllerCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

VMEControllerCallback::VMEControllerCallback(const NSMNode& node)
  : RCCallback(node)
{
}

VMEControllerCallback::~VMEControllerCallback() throw()
{

}

bool VMEControllerCallback::boot() throw()
{
  LogFile::debug("BOOT");
  return true;
}

bool VMEControllerCallback::load() throw()
{
  LogFile::debug("LOAD");
  return true;
}

bool VMEControllerCallback::start() throw()
{
  LogFile::debug("START");
  return true;
}

bool VMEControllerCallback::stop() throw()
{
  LogFile::debug("STOP");
  return true;
}

bool VMEControllerCallback::recover() throw()
{
  LogFile::debug("RECOVER");
  return true;
}

