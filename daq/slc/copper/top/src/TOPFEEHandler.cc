#include "daq/slc/copper/top/TOPFEEHandler.h"
#include "daq/slc/copper/top/ConfigureBoardStack.h"
#include "daq/slc/copper/top/PrepareBoardstackData.h"
#include "daq/slc/copper/top/PrepareBoardstackFe.h"
#include "daq/slc/copper/top/B2L_defs.h"

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/copper/HSLB.h>

using namespace Belle2;

bool TOPHandlerLookback::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPHandlerLookback::handleSetInt(const int val)
{
  int useBoardStack = 0;
  m_callback.get(StringUtil::form("top[%d].useBsInConfig", m_hslb.get_finid()), useBoardStack);
  if (useBoardStack != 0)PrepBoardstackData::SetLookback(m_hslb, val);
  m_callback.log(LogFile::DEBUG, StringUtil::form("lookback value changed to %d on scrod %d", val, m_hslb.get_finid()));
  return NSMVHandlerInt::handleSetInt(val);
}


bool TOPHandlerFEMode::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPHandlerFEMode::handleSetInt(const int val)
{
  int useBoardStack = 0;
  m_callback.get(StringUtil::form("top[%d].useBsInConfig", m_hslb.get_finid()), useBoardStack);
  if (useBoardStack != 0) PrepBoardstackData::SetFEMode(m_hslb, val);
  std::string femodeString = "UNDEFINED";
  if (val > -1 && val < 4) femodeString = featureExtModeList[val];
  m_callback.log(LogFile::DEBUG, StringUtil::form("feature extraction mode changed to " + femodeString + " on scrod %d",
                                                  m_hslb.get_finid()));
  return NSMVHandlerInt::handleSetInt(val);
}

bool TOPConfigureBS::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPConfigureBS::handleSetInt(const int val)
{
  int useBoardStack = 0;
  m_callback.get(StringUtil::form("top[%d].useBsInConfig", m_hslb.get_finid()), useBoardStack);

  if (useBoardStack != 0 && val == 1) {
    m_callback.log(LogFile::DEBUG, StringUtil::form("Starting Configuration on %d", m_hslb.get_finid()));
    ConfigBoardstack::ConfigureBoardStack(m_hslb, m_callback);
  } else {
    m_callback.log(LogFile::DEBUG, StringUtil::form("configuration disabled for BS %d", m_hslb.get_finid()));
  }
  return NSMVHandlerInt::handleSetInt(0);
}

bool TOPPrepareData::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPPrepareData::handleSetInt(const int val)
{
  int useBoardStack = 0;
  m_callback.get(StringUtil::form("top[%d].useBsInConfig", m_hslb.get_finid()), useBoardStack);
  if (useBoardStack != 0 && val == 1) {
    m_callback.log(LogFile::DEBUG, StringUtil::form("Starting Data Taking Preparation for BS %d", m_hslb.get_finid()));
    PrepBoardstackData::PrepareBoardStack(m_hslb, m_callback);
  } else {
    m_callback.log(LogFile::DEBUG, StringUtil::form("configuration disabled for BS %d", m_hslb.get_finid()));
  }
  return NSMVHandlerInt::handleSetInt(0);
}

bool TOPPrepareFE::handleGetInt(int& value)
{
  return NSMVHandlerInt::handleGetInt(value);
}

bool TOPPrepareFE::handleSetInt(const int val)
{
  int useBoardStack = 0;
  m_callback.get(StringUtil::form("top[%d].useBsInConfig", m_hslb.get_finid()), useBoardStack);
  if (useBoardStack != 0 && val == 1) {
    m_callback.log(LogFile::DEBUG, StringUtil::form("Starting Feature Extraction Preparation for BS %d", m_hslb.get_finid()));
    PrepBoardstackFE::PrepareBoardStack(m_hslb, m_callback);
  } else {
    m_callback.log(LogFile::DEBUG, StringUtil::form("configuration disabled for BS %d", m_hslb.get_finid()));
  }
  return NSMVHandlerInt::handleSetInt(0);
}
