#include "daq/slc/copper/top/TOPFEEHandler.h"
#include "daq/slc/copper/top/PrepareBoardstackData.h"
#include "daq/slc/copper/top/B2L_defs.h"

#include <daq/slc/runcontrol/RCCallback.h>


using namespace Belle2;

bool TOPHandlerLookback::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPHandlerLookback::handleSetInt(const int val)
{
  PrepBoardstackData::SetLookback(m_hslb, val);
  m_callback.log(LogFile::DEBUG, StringUtil::form("lookback value changed to %d", val));
  return NSMVHandlerInt::handleSetInt(val);
}


bool TOPHandlerFEMode::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPHandlerFEMode::handleSetInt(const int val)
{
  PrepBoardstackData::SetFEMode(m_hslb, val);
  std::string femodeString = "UNDEFINED";
  if (val > -1 && val < 4) femodeString = featureExtModeList[val];
  m_callback.log(LogFile::DEBUG, "feature extraction mode changed to " + femodeString);
  return NSMVHandlerInt::handleSetInt(val);
}
