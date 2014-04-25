#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

void DQMViewCallback::init() throw()
{
  m_master.setCallback(this);
  m_master.init();
  m_master.boot();
  m_master.setState(RCState::INITIAL_S);
  PThread(new DQMPackageUpdater(this, m_master));
}

bool DQMViewCallback::boot() throw()
{
  m_master.setState(RCState::CONFIGURED_S);
  return true;
}

bool DQMViewCallback::load() throw()
{
  m_master.setState(RCState::READY_S);
  return true;
}

bool DQMViewCallback::start() throw()
{
  NSMMessage& nsm(getMessage());
  m_expno = nsm.getParam(0);
  m_runno = nsm.getParam(1);
  m_master.setRunNumbers(m_expno, m_runno);
  m_master.setState(RCState::RUNNING_S);
  return true;
}

bool DQMViewCallback::stop() throw()
{
  LogFile::debug("creating new DQM records for run # %04d.%06d",
                 (int)m_expno, (int)m_runno);
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  std::vector<DQMFileReader>& reader_v(m_master.getReaders());
  for (size_t i = 0; i < reader_v.size(); i++) {
    reader_v[i].dump(dumppath, m_expno, m_runno);
  }
  m_master.setState(RCState::READY_S);
  return true;
}

bool DQMViewCallback::pause() throw()
{
  m_master.setState(RCState::PAUSED_S);
  return true;
}

bool DQMViewCallback::abort() throw()
{
  m_master.setState(RCState::INITIAL_S);
  return true;//_master.abort();
}

bool DQMViewCallback::recover() throw()
{
  return abort() && boot() && load();
}

