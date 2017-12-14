#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"
#include "daq/slc/system/Time.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <signal.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <cstdarg>
#include <cctype>

using namespace Belle2;

NSMCallback::NSMCallback(const int timeout) throw()
  : AbstractNSMCallback(timeout)
{
  reg(NSMCommand::OK);
  reg(NSMCommand::ERROR);
  reg(NSMCommand::LOG);
  reg(NSMCommand::LOGSET);
  reg(NSMCommand::LOGGET);
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

void NSMCallback::reply(const NSMMessage& msg)
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
}

void NSMCallback::log(LogFile::Priority pri, const char* format, ...)
{
  va_list ap;
  char ss[1024 * 10];
  va_start(ap, format);
  vsprintf(ss, format, ap);
  va_end(ap);
  log(pri, std::string(ss));
}

void NSMCallback::log(LogFile::Priority pri, const std::string& msg)
{
  LogFile::put(pri, msg);
  try {
    if (getLogNode().getName().size() > 0) {
      NSMCommunicator::send(NSMMessage(getLogNode().getName(),
                                       DAQLogMessage(getNode().getName(),
                                                     pri, msg)));
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

bool NSMCallback::perform(NSMCommunicator& com) throw()
{
  const NSMMessage& msg(com.getMessage());
  const NSMCommand cmd = msg.getRequestName();
  if (cmd == NSMCommand::OK || cmd == NSMCommand::ERROR ||
      cmd == NSMCommand::FATAL) {
    if (msg.getLength() > 0) {
      if (cmd == NSMCommand::OK) {
        ok(msg.getNodeName(), msg.getData());
      } else if (cmd == NSMCommand::ERROR) {
        error(msg.getNodeName(), msg.getData());
      } else if (cmd == NSMCommand::FATAL) {
        fatal(msg.getNodeName(), msg.getData());
      }
    }
    return true;
  } else if (cmd == NSMCommand::LOG) {
    int id = com.getNodeIdByName(msg.getNodeName());
    DAQLogMessage lmsg;
    lmsg.setId(id);
    lmsg.setNodeName(msg.getNodeName());
    if (msg.getNParams() > 0) {
      lmsg.setPriority((LogFile::Priority)msg.getParam(0));
    }
    if (msg.getNParams() > 1) lmsg.setDate(msg.getParam(1));
    lmsg.setMessage(msg.getLength() > 0 ? msg.getData() : "");
    logset(lmsg);
    return true;
  } else if (cmd == NSMCommand::LOGSET) {
    if (msg.getNParams() > 2 && msg.getLength() > 0) {
      DAQLogMessage lmsg;
      if (msg.getNParams() > 0) {
        lmsg.setPriority((LogFile::Priority)msg.getParam(0));
      }
      if (msg.getParam(1) > 0) lmsg.setDate(msg.getParam(1));
      lmsg.setId(msg.getParam(2));
      lmsg.setNodeName(com.getNodeNameById(msg.getParam(2)));
      lmsg.setMessage(msg.getData());
      logset(lmsg);
    }
    return true;
  } else if (cmd == NSMCommand::LOGGET) {
    std::string nodename = msg.getNodeName();
    LogFile::Priority pri = (LogFile::Priority) msg.getParam(0);
    logget(nodename, pri);
    return true;
  } else if (cmd == NSMCommand::VGET) {
    if (msg.getLength() > 0) {
      vget(msg.getNodeName(), msg.getData());
    }
    return true;
  } else if (cmd == NSMCommand::VSET) {
    if (msg.getLength() > 0) {
      NSMVar var;
      readVar(msg, var);
      vset(com, var);
    }
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
  NSMVar var_out = var;
  if (var_out.getNode().size() == 0) {
    var_out.setNode(getNode().getName());
  }

  if (m_node_v_m.find(vname) != m_node_v_m.end()) {
    try {
      NSMNodeMap& node_v(m_node_v_m[vname]);
      for (NSMNodeMap::iterator inode = node_v.begin();
           inode != node_v.end();) {
        NSMNode& node(inode->second);
        if (NSMCommunicator::send(NSMMessage(node, var_out))) {
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

void NSMCallback::vget(const std::string& nodename, const std::string& vname) throw()
{
  NSMNode node(nodename);
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
        if (node_v.find(nodename) == node_v.end()) {
          node_v.insert(NSMNodeMap::value_type(nodename, NSMNode(nodename)));
          std::string filename = ("/tmp/nsmvget." + StringUtil::tolower(getNode().getName()));
          std::ofstream fout(filename.c_str(), std::ios::app);
          fout << nodename << " " << vname << std::endl;
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
        if ((result = handler.handleSet(var)) &&
            (var.getNode().size() == 0 || var.getNode() == getNode().getName())) {
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
    LogFile::warning(e.what());
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

