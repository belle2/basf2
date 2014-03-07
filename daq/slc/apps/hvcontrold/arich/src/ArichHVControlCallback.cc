#include "daq/slc/apps/hvcontrold/arich/ArichHVControlCallback.h"

#include "daq/slc/apps/hvcontrold/arich/ArichHVCommunicator.h"
#include "daq/slc/apps/hvcontrold/arich/ArichHVMonitor.h"

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>

using namespace Belle2;

ArichHVControlCallback::~ArichHVControlCallback() throw()
{
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    delete m_comm_v[i];
  }
}

void ArichHVControlCallback::initialize() throw()
{
  ConfigFile config("hvcontrol/arich");
  const int ncrates = config.getInt("arich.hv.ncrates");
  for (int n = 0; n < ncrates; n++) {
    const std::string host = config.get(Belle2::form("arich.hv.%d.host", n));
    int port = config.getInt(Belle2::form("arich.hv.%d.port", n));
    ArichHVCommunicator* comm = new ArichHVCommunicator(this, n, host, port);
    PThread(new ArichHVMonitor(comm));
    m_comm_v.push_back(comm);
  }
}

bool ArichHVControlCallback::configure() throw()
{
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    m_comm_v[i]->configure();
  }
  return true;
}

bool ArichHVControlCallback::turnon() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->turnon();
  }
  return success && waitStable();
}

bool ArichHVControlCallback::turnoff() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->turnoff();
  }
  return success && waitStable();
}

bool ArichHVControlCallback::standby() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->standby();
  }
  return success && waitStable();
}

bool ArichHVControlCallback::standby2() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->standby2();
  }
  return success && waitStable();
}

bool ArichHVControlCallback::standby3() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->standby3();
  }
  return success && waitStable();
}

bool ArichHVControlCallback::peak() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->peak();
  }
  return success && waitStable();
}

bool ArichHVControlCallback::waitStable() throw()
{
  bool is_error = false;
  while (true) {
    bool is_stable = true;
    for (HVChannelInfoList::iterator it = m_info.getChannelList().begin();
         it != m_info.getChannelList().end(); it++) {
      HVChannelStatus& status(it->getStatus());
      HVState state = status.getState();
      if (!state.isStable()) is_stable = false;
      if (state.isError()) is_error = true;
    }
    if (is_stable || is_error) break;
    sleep(2);
  }
  return !is_error;
}
