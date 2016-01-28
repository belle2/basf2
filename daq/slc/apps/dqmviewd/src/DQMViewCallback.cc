#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"

#include <daq/slc/apps/dqmviewd/SocketAcceptor.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

DQMViewCallback::DQMViewCallback(const NSMNode& node, ConfigFile& config)
  : NSMCallback(1), m_config(config)
{
  setNode(node);
  const std::string host = m_config.get("dqm.host");
  const int port = m_config.getInt("dqm.port");
  int ntmap = m_config.getInt("dqm.ntmap");
  for (int i = 0; i < ntmap; i++) {
    std::string vname = StringUtil::form("dqm[%d]", i);
    const std::string map_name = m_config.get(vname + ".file");
    std::string mapfile = /*map_path + "/" +*/ map_name;
    add(new NSMVHandlerText(vname + ".map", true, false, map_name));
    add(new NSMVHandlerText(vname + ".mapfile", true, false, mapfile));
    m_memory.push_back(new DqmMemFile(mapfile));
  }
  PThread(new SocketAcceptor(host, port, this));
}

void DQMViewCallback::init(NSMCommunicator&) throw()
{
  const std::string map_path = m_config.get("dqm.tmapdir");
  const std::string host = m_config.get("dqm.host");
  const int port = m_config.getInt("dqm.port");
  int ntmap = m_config.getInt("dqm.ntmap");
  add(new NSMVHandlerText("tmapdir", true, false, map_path));
  add(new NSMVHandlerText("host", true, false, host));
  add(new NSMVHandlerInt("port", true, false, port));
  for (int i = 0; i < ntmap; i++) {
    std::string vname = StringUtil::form("dqm[%d]", i);
    const std::string map_name = m_config.get(vname + ".file");
    std::string mapfile = /*map_path + "/" +*/ map_name;
    add(new NSMVHandlerText(vname + ".map", true, false, map_name));
    add(new NSMVHandlerText(vname + ".mapfile", true, false, mapfile));
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
  std::vector<TH1*> hist;
  for (size_t i = 0; i < m_memory.size(); i++) {
    TMemFile* file = m_memory[i]->LoadMemFile();
    file->cd();
    TIter next(file->GetListOfKeys());
    TKey* key = NULL;
    while ((key = (TKey*)next())) {
      hist.push_back((TH1*)key->ReadObj());
    }
  }
  if (hist.size() > 0) {
    m_hist = hist;
    notify();
  }
  unlock();
}

