#include "daq/slc/apps/mwreaderd/MWReaderCallback.h"
#include "daq/slc/apps/mwreaderd/mwreader.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

using namespace Belle2;

MWReaderCallback::MWReaderCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

MWReaderCallback::~MWReaderCallback() throw()
{
}

void MWReaderCallback::init(NSMCommunicator&) throw()
{
  allocData("MW100", "mwreader", mwreader_revision);
}

void MWReaderCallback::timeout(NSMCommunicator&) throw()
{
  try {
    NSMData& data(getData());
    if (!m_reader.get() && data.isAvailable()) {
      LogFile::debug("set reader");
      m_reader.set((mwreader*)data.get());
      LogFile::debug("init reader");
      m_reader.init();
    }
    if (m_reader.get()) {
      LogFile::debug("update reader");
      m_reader.update();
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
}

