#include "daq/slc/apps/filemond/FilemonCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

using namespace Belle2;

FilemonCallback::FilemonCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

FilemonCallback::~FilemonCallback() throw()
{
}

void FilemonCallback::init(NSMCommunicator&) throw()
{
}

void FilemonCallback::timeout(NSMCommunicator&) throw()
{
}

