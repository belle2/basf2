#include "daq/slc/apps/rc2eb1d/Rc2eb1Callback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <stdlib.h>

using namespace Belle2;

Rc2eb1Callback::Rc2eb1Callback(const std::string& name, ConfigFile& conf,
                               int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
  m_rcnode = NSMNode(conf.get("rcnode"));
  m_eb1 = NSMNode(conf.get("eb1"));
  StringList list = StringUtil::split(conf.get("nodes"), ',');
  for (size_t i = 0; i < list.size(); i++) {
    node_hosts node;
    StringList s = StringUtil::split(list[i], '=');
    node.name = s[0];
    node.hosts = StringUtil::split(s[1], ':');
    m_nodes.push_back(node);
  }
}

Rc2eb1Callback::~Rc2eb1Callback() throw()
{
}

void Rc2eb1Callback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  if (var.getNode() == m_rcnode.getName()) {
    LogFile::info(var.getNode() + "." + var.getName());
    if (var.getType() == NSMVar::INT) {
      for (size_t i = 0; i < m_nodes.size(); i++) {
        if (var.getName() == (m_nodes[i].name + ".used")) {
          std::vector<std::string>& hosts(m_nodes[i].hosts);
          for (size_t j = 0; j < hosts.size(); j++) {
            try {
              LogFile::info(hosts[j] + ".used");
              set(m_eb1, hosts[j] + ".used", var.getInt());
            } catch (const IOException& e) {
              LogFile::error(e.what());
            }
          }
        }
      }
      return;
    }
    NSMCallback::vset(com, var);
  } else {
    NSMCallback::vset(com, var);
  }
}

void Rc2eb1Callback::init(NSMCommunicator&) throw()
{
}

void Rc2eb1Callback::timeout(NSMCommunicator&) throw()
{
  for (size_t i = 0; i < m_nodes.size(); i++) {
    int used;
    LogFile::info(m_nodes[i].name + ".used");
    try {
      get(m_rcnode, m_nodes[i].name + ".used", used);
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
}

