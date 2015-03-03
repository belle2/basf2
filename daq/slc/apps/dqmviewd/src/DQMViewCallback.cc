#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"

#include <daq/slc/apps/dqmviewd/SocketAcceptor.h>

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

DQMViewCallback::DQMViewCallback(const NSMNode& node,
                                 ConfigFile& config)
  : NSMCallback(5), m_config(config)
{
  setNode(node);
  m_runcontrol = NSMNode(m_config.get("dqm.runcontrol"));
  const std::string map_path = m_config.get("dqm.tmap.dir");
  const std::string host = m_config.get("dqm.host");
  const int port = m_config.getInt("dqm.port");
  StringList dqmlist = StringUtil::split(m_config.get("dqm.tmap.list"), ',');
  m_dump_path = m_config.get("dqm.rootfiles.dir");
  for (size_t i = 0; i < dqmlist.size(); i++) {
    const std::string pack_name = m_config.get(StringUtil::form("dqm.%s.name", dqmlist[i].c_str()));
    const std::string map_name = m_config.get(StringUtil::form("dqm.%s.file", dqmlist[i].c_str()));
    std::string mapfile = map_path + "/" + map_name;
    addReader(pack_name, mapfile);
  }
  PThread(new SocketAcceptor(host, port, this));

}

void DQMViewCallback::init(NSMCommunicator&) throw()
{
  const std::string map_path = m_config.get("dqm.tmap.dir");
  const std::string host = m_config.get("dqm.host");
  const int port = m_config.getInt("dqm.port");
  StringList dqmlist = StringUtil::split(m_config.get("dqm.tmap.list"), ',');
  add(new NSMVHandlerText("map_dir", true, false, map_path));
  add(new NSMVHandlerText("host", true, false, host));
  add(new NSMVHandlerInt("port", true, false, port));
  add(new NSMVHandlerInt("npackages", true, false, dqmlist.size()));
  add(new NSMVHandlerText("runcontrol", true, false, m_runcontrol.getName()));
  for (size_t i = 0; i < dqmlist.size(); i++) {
    const std::string pack_name = m_config.get(StringUtil::form("dqm.%s.name", dqmlist[i].c_str()));
    const std::string map_name = m_config.get(StringUtil::form("dqm.%s.file", dqmlist[i].c_str()));
    std::string mapfile = map_path + "/" + map_name;
    std::string vname = StringUtil::form("package[%d]", (int)i);
    add(new NSMVHandlerText(vname + ".name", true, false, pack_name));
    add(new NSMVHandlerText(vname + ".map", true, false, map_name));
    add(new NSMVHandlerText(vname + ".mapfile", true, false, mapfile));
  }
  std::string ss;
  try {
    get(m_runcontrol, "rcstate", ss);
    get(m_runcontrol, "expno", m_expno);
    get(m_runcontrol, "runno", m_runno);
    get(m_runcontrol, "subno", m_subno);
  } catch (const TimeoutException& e) {
    LogFile::warning("runcontrol is unreachable");
  }
  update();
}

void DQMViewCallback::timeout(NSMCommunicator&) throw()
{
  update();
}

void DQMViewCallback::update() throw()
{
  lock();
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    DQMFileReader& reader(m_reader_v[i]);
    std::string filename = reader.getFileName();
    if (!reader.isReady() && reader.init(i, *this)) {
      LogFile::debug("Hist entries was found in %s", filename.c_str());
    }
    if (reader.isReady()) reader.update();
  }
  notify();
  unlock();
}

void DQMViewCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  if (var.getName() == "rcstate") {
    try {
      get(m_runcontrol, "expno", m_expno);
      get(m_runcontrol, "runno", m_runno);
      get(m_runcontrol, "subno", m_subno);
    } catch (const TimeoutException& e) {
      LogFile::error("runcontrol is unreachable");
    }
    std::string state = var.getText();
    if (state == "STOPPING") {
      LogFile::debug("run # %04d.%06d.%03d %s",
                     m_expno, m_runno, m_subno, state.c_str());
    }
    return;
  }
  NSMCallback::vset(com, var);
}

bool DQMViewCallback::record() throw()
{
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    m_reader_v[i].dump(m_dump_path, m_expno, m_runno);
  }
  getNode().setState(RCState::READY_S);
  return true;
}


