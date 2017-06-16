#include "daq/slc/copper/top/TOPFEE.h"
#include "daq/slc/copper/FEEHandler.h"
#include "daq/slc/copper/top/TOPFEEHandler.h"

#include "daq/slc/copper/top/B2L_common.h"
#include "daq/slc/copper/top/ConfigureBoardStack.h"
#include "daq/slc/copper/top/PrepareBoardstackData.h"
#include "daq/slc/copper/top/PrepareBoardstackFe.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>


using namespace Belle2;
using namespace std;

TOPFEE::TOPFEE()
{
  m_numberOfCarriers.clear();
  m_statusMonitor.clear();
  m_initialConfigurationDone = false;
}

void TOPFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  int csr = 0;
  hslb.reset_b2l(csr);

  int numberOfCarriers = GetNumberOfCarriers(hslb.get_finid());
  m_numberOfCarriers[hslb.get_finid()] = numberOfCarriers;
  if (numberOfCarriers > -1) {
    BoardStackStatus statusMonitor;
    statusMonitor.SetNumberOfCarriers(numberOfCarriers);
    statusMonitor.InitNSMCallbacks(hslb, callback);
    m_statusMonitor[hslb.get_finid()] = statusMonitor;
  }
  ConfigBoardstack::InitBoardStackCallbacks(hslb, callback);
  PrepBoardstackFE::InitCallbacks(hslb, callback);

  ConfigBoardstack::LoadDefaultRegisterValues(hslb);
  ConfigBoardstack::UpdateRegisterFromDatabase(obj);
  //ConfigBoardstack::PrintRegisterValueMap();

  //callbacks to directly change registers on the board stacks
  callback.add(new TOPHandlerLookback(StringUtil::form("top[%d].Lookback", hslb.get_finid()), callback, hslb, *this, 44));
  callback.add(new TOPHandlerFEMode(StringUtil::form("top[%d].ScrodfeMode", hslb.get_finid()), callback, hslb, *this, 3));
}

void TOPFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  //only know how to call this from command line
}

void TOPFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  hslb.writefn32(HSREGL_RESET, 0xffffff);
  //not sure if initial configuration should already be done in Init function
  if (!m_initialConfigurationDone) {
    callback.log(LogFile::DEBUG, "Boot: Starting Configuration");
    ConfigBoardstack::ConfigureBoardStack(hslb, callback);
    callback.log(LogFile::DEBUG, "Boot: Starting Prep Data");
    PrepBoardstackData::PrepareBoardStack(hslb, callback);
    callback.log(LogFile::DEBUG, "Boot: Taking Pedestals");
    PrepBoardstackFE::PrepareBoardStack(hslb, callback);
    callback.log(LogFile::DEBUG, "Initial Configuration and Pedestal Accquisiton done.");
  }
  m_initialConfigurationDone = true;

  callback.log(LogFile::DEBUG, "Load: Starting Prep Data");
  PrepBoardstackData::PrepareBoardStack(hslb, callback);
}

void TOPFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  int id = hslb.get_finid();
  map<int, BoardStackStatus>::iterator it = m_statusMonitor.find(id);
  if (it != m_statusMonitor.end()) {
    it->second.UpdateNSMCallbacks(hslb, callback);
  } else {
    callback.log(LogFile::DEBUG, StringUtil::form("Boardstack %d not found", id));
  }
}


extern "C" {
  void* getTOPFEE()
  {
    return new Belle2::TOPFEE();
  }
}
