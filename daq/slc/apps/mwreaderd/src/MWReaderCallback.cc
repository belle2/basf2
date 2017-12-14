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
  static unsigned long long count = 0;
  try {
    NSMData& data(getData());
    if (!m_reader.get() && data.isAvailable()) {
      m_reader.set((mwreader*)data.get());
      m_reader.init();
    }
    if (m_reader.get()) {
      m_reader.update();
      mwreader* reader = (mwreader*)data.get();
      const int nitem = reader->nitem;
      for (int i = 0; i < nitem; i++) {
        mwreader::mwdata& mdata(reader->data[i]);
        std::string vname = StringUtil::form("data[%d].", i);
        if (count == 0) {
          add(new NSMVHandlerText(vname + "unit", true, false, mdata.unit));
          add(new NSMVHandlerText(vname + "alarm", true, false, mdata.alarm));
          add(new NSMVHandlerText(vname + "cond", true, false, mdata.cond));
          add(new NSMVHandlerFloat(vname + "value", true, false, mdata.value));
          add(new NSMVHandlerInt(vname + "chan", true, false, mdata.chan));
        } else {
          set(vname + "unit", mdata.unit);
          // Send error (or warning) when string has value
          set(vname + "alarm", mdata.alarm);
          set(vname + "cond", mdata.cond);
          set(vname + "value", mdata.value);
          set(vname + "chan", mdata.chan);
        }
      }
      LogFile::debug("count=%llu", count);
      count++;
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
}

