#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"
#include "daq/slc/system/Time.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <signal.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace Belle2;

NSMCallback::NSMCallback(const int timeout) throw()
  : AbstractNSMCallback(timeout)
{
  reg(NSMCommand::OK);
  reg(NSMCommand::ERROR);
  reg(NSMCommand::LOG);
  reg(NSMCommand::VGET);
  reg(NSMCommand::VSET);
  reg(NSMCommand::VREPLY);
  reg(NSMCommand::VLISTGET);
  reg(NSMCommand::VLISTSET);
  reg(NSMCommand::DATAGET);
  reg(NSMCommand::DATASET);
}

void NSMCallback::addNode(const NSMNode& node)
{
  std::string name = node.getName();
  if (m_nodes.find(name) == m_nodes.end()) {
    m_nodes.insert(NSMNodeMap::value_type(name, node));
  }
}

bool NSMCallback::reply(const NSMMessage& msg)
throw(NSMHandlerException)
{
  NSMMessage msg_out = msg;
  for (NSMNodeMap::iterator it = m_nodes.begin();
       it != m_nodes.end();) {
    NSMNode& node(it->second);
    msg_out.setNodeName(node.getName());
    if (NSMCommunicator::send(msg_out)) {
      it++;
    } else {
      m_nodes.erase(it++);
    }
  }
  return true;
}

bool NSMCallback::perform(NSMCommunicator& com) throw()
{
  const NSMMessage& msg(com.getMessage());
  const NSMCommand cmd = msg.getRequestName();
  if (cmd == NSMCommand::OK || cmd == NSMCommand::ERROR) {
    if (msg.getLength() > 0) {
      if (cmd == NSMCommand::OK) {
        ok(msg.getNodeName(), msg.getData());
      } else if (cmd == NSMCommand::ERROR) {
        error(msg.getNodeName(), msg.getData());
      }
    }
    return true;
  } else if (cmd == NSMCommand::LOG) {
    DAQLogMessage lmsg;
    if (lmsg.read(msg)) {
      log(msg.getNodeName(), lmsg, (bool)msg.getParam(2));
    }
    return true;
  } else if (cmd == NSMCommand::VGET) {
    std::string vname = msg.getData();
    vget(com, vname);
    return true;
  } else if (cmd == NSMCommand::VSET) {
    NSMVar var;
    readVar(msg, var);
    vset(com, var);
    return true;
  } else if (cmd == NSMCommand::VREPLY) {
    vreply(com, msg.getData(), msg.getParam(0));
    return true;
  } else if (cmd == NSMCommand::VLISTGET) {
    vlistget(com);
    return true;
  } else if (cmd == NSMCommand::VLISTSET) {
    vlistset(com);
    return true;
  } else if (cmd == NSMCommand::DATAGET) {
    nsmdataget(com);
    return true;
  } else if (cmd == NSMCommand::DATASET) {
    const NSMMessage& msg(com.getMessage());
    if (msg.getNParams() > 1 && msg.getLength() > 0) {
      const int* pars = (const int*)msg.getParams();
      int revision = pars[0];
      int size = pars[1];
      const char* pdata = msg.getData();
      StringList ss = StringUtil::split(pdata, '\n');
      if (ss.size() >= 2) {
        std::string name = ss[0];
        std::string format = ss[1];
        pdata += strlen(pdata) + 1;
        for (NSMDataMap::iterator it = m_datas.begin(); it != m_datas.end(); it++) {
          NSMData& data(it->second);
          if (data.isAvailable() &&
              data.getName() == name && data.getFormat() == format &&
              data.getRevision() == revision && data.getSize() == size) {
            memcpy(data.get(), pdata, size);
            nsmdataset(com, data);
          }
        }
      }
    }
    return true;
  }
  return false;
}

void NSMCallback::notify(const NSMVar& var) throw()
{
  std::string vname = var.getName();
  if (m_node_v_m.find(vname) != m_node_v_m.end()) {
    try {
      NSMNodeMap& node_v(m_node_v_m[vname]);
      for (NSMNodeMap::iterator inode = node_v.begin();
           inode != node_v.end();) {
        NSMNode& node(inode->second);
        if (NSMCommunicator::send(NSMMessage(node, var))) {
          inode++;
        } else {
          node_v.erase(inode++);
        }
      }
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
}

void NSMCallback::nsmdataset(NSMCommunicator&, NSMData&) throw()
{
}

void NSMCallback::nsmdataget(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  if (msg.getNParams() > 1) {
    const int* pars = (const int*)msg.getParams();
    int revision = pars[0];
    int size = pars[1];
    const char* pdata = (const char*)msg.getData();
    StringList ss = StringUtil::split(pdata, '\n');
    if (ss.size() < 2) return;
    std::string name = ss[0];
    std::string format = ss[1];
    NSMData& data(getData());
    if (data.isAvailable() &&
        data.getName() == name && data.getFormat() == format &&
        data.getRevision() == revision && data.getSize() == size) {
      NSMCommunicator::send(NSMMessage(NSMNode(msg.getNodeName()), data));
      return;
    }
  }
}

void NSMCallback::vget(NSMCommunicator& com, const std::string& vname) throw()
{
  NSMMessage msg(com.getMessage());
  NSMNode node(msg.getNodeName());
  NSMVar var(vname);
  try {
    for (size_t i = 0; i < m_handler.size(); i++) {
      NSMVHandler& handler(*m_handler[i]);
      if (var.getName() == handler.getName() &&
          handler.useGet() && handler.handleGet(var)) {
        var.setNode(getNode().getName());
        var.setName(vname);
        var.setId(i + 1);
        var.setRevision(getRevision());
        NSMCommunicator::send(NSMMessage(node, var));
        if (m_node_v_m.find(vname) == m_node_v_m.end()) {
          m_node_v_m.insert(NSMNodeMapMap::value_type(vname, NSMNodeMap()));
        }
        NSMNodeMap& node_v(m_node_v_m[vname]);
        std::string nodename = msg.getNodeName();
        if (node_v.find(nodename) == node_v.end()) {
          node_v.insert(NSMNodeMap::value_type(nodename, NSMNode(nodename)));
        }
      }
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void NSMCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  try {
    NSMMessage msg(com.getMessage());
    for (size_t i = 0; i < m_handler.size(); i++) {
      NSMVHandler& handler(*m_handler[i]);
      bool result = false;
      if (var.getName() == handler.getName() &&
          (var.getNode() == handler.getNode() ||
           (var.getNode().size() == 0 && handler.getNode() == getNode().getName()) ||
           (handler.getNode().size() == 0 && var.getNode() == getNode().getName())) &&
          handler.useSet()) {
        NSMNode node(msg.getNodeName());
        if ((result = handler.handleSet(var))) {
          notify(var);
        }
        if (var.getNode().size() == 0) {
          NSMCommunicator::send(NSMMessage(node, NSMCommand::VREPLY,
                                           result, var.getName()));
        }
      }
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void NSMCallback::vlistget(NSMCommunicator& com) throw()
{
  std::stringstream ss;
  int count = 0;
  for (size_t i = 0; i < m_handler.size(); i++) {
    NSMVHandler& handler(*m_handler[i]);
    if (handler.getNode().size() == 0) {
      ss << handler.getName() << ":"
         << handler.get().getTypeLabel() << ":"
         << (int)handler.useGet() << ":"
         << (int)handler.useSet() << ":"
         << i << "\n";
      count++;
    }
  }
  std::string nodename = com.getMessage().getNodeName();
  NSMNode node(nodename);
  try {
    NSMCommunicator::send(NSMMessage(node, NSMCommand::VLISTSET,
                                     count, ss.str()));
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

struct vlistentry_t {
  std::string name;
  int id;
  std::string type;
  bool useGet;
  bool useSet;
};

void NSMCallback::vlistset(NSMCommunicator& com) throw()
{
  std::string data = com.getMessage().getData();
  StringList s = StringUtil::split(data, '\n');
  std::vector<vlistentry_t> vlist;
  size_t length = 0;
  for (size_t i = 0; i < s.size(); i++) {
    StringList ss = StringUtil::split(s[i], ':');
    if (ss.size() > 4) {
      vlistentry_t en = { ss[0], atoi(ss[4].c_str()),
                          ss[1], ss[2] == "1", ss[3] == "1"
                        };
      vlist.push_back(en);
      if (length < ss[0].size()) length = ss[0].size();
    }
  }
  for (size_t i = 0; i < vlist.size(); i++) {
    vlistentry_t& en(vlist[i]);
    std::cout << StringUtil::form(StringUtil::form("%%-%ds ", length).c_str(), en.name.c_str())
              << " : " << en.type << " "
              << (en.useGet ? "get " : "")
              << (en.useSet ? "set " : "")  << std::endl;
  }
}

void NSMCallback::alloc_open(NSMCommunicator& com) throw()
{
  try {
    if (!m_data.isAvailable() && m_data.getName().size() > 0 &&
        m_data.getFormat().size() > 0 && m_data.getRevision() > 0) {
      m_data.allocate(com);
    }
  } catch (const NSMHandlerException& e) {
  }
  for (NSMDataMap::iterator it = m_datas.begin();
       it != m_datas.end(); it++) {
    NSMData& data(it->second);
    try {
      if (!data.isAvailable() && data.getName().size() > 0 &&
          data.getFormat().size() > 0) {
        data.open(com);
      }
    } catch (const NSMHandlerException& e) {
    }
  }
}
