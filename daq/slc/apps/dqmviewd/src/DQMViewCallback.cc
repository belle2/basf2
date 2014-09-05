#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include <daq/slc/apps/dqmviewd/SocketAcceptor.h>

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

DQMViewCallback::DQMViewCallback(const NSMNode& node,
                                 ConfigFile& config)
  : NSMCallback(node, 10), m_config(config)
{
  add(RCCommand::START);
  add(RCCommand::STOP);
}

void DQMViewCallback::init() throw()
{
  const std::string map_path = m_config.get("dqm.tmap.dir");
  const std::string hostname = m_config.get("dqm.host");
  const int port = m_config.getInt("dqm.port");
  StringList dqmlist = StringUtil::split(m_config.get("dqm.tmap.list"), ',');
  for (size_t i = 0; i < dqmlist.size(); i++) {
    const std::string pack_name = m_config.get(StringUtil::form("dqm.%s.name", dqmlist[i].c_str()));
    const std::string map_name = m_config.get(StringUtil::form("dqm.%s.file", dqmlist[i].c_str()));
    std::string mapfile = map_path + "/" + map_name;
    addReader(pack_name, mapfile);
  }
  PThread(new SocketAcceptor(hostname, port, this));
  getNode().setState(RCState::READY_S);
}

bool DQMViewCallback::perform(const NSMMessage& msg) throw()
{
  if (NSMCallback::perform(msg)) return true;
  const RCCommand cmd = msg.getRequestName();
  if (cmd == RCCommand::START) {
    return start();
  } else if (cmd == RCCommand::STOP) {
    return stop();
  }
  return true;
}

void DQMViewCallback::timeout() throw()
{
  lock();
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    DQMFileReader& reader(m_reader_v[i]);
    std::string filename = reader.getFileName();
    if (!reader.isReady() && reader.init()) {
      LogFile::debug("Hist entries was found in %s", filename.c_str());
    }
    if (reader.isReady()) reader.update();
  }
  notify();
  unlock();
}

bool DQMViewCallback::start() throw()
{
  NSMMessage& nsm(getMessage());
  m_expno = nsm.getParam(0);
  m_runno = nsm.getParam(1);
  getNode().setState(RCState::RUNNING_S);
  return true;
}

bool DQMViewCallback::stop() throw()
{
  LogFile::debug("creating new DQM records for run # %04d.%06d",
                 (int)m_expno, (int)m_runno);
  const std::string dumppath = m_config.get("dqm.rootfiles.dir");
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    m_reader_v[i].dump(dumppath, m_expno, m_runno);
  }
  getNode().setState(RCState::READY_S);
  return true;
}


