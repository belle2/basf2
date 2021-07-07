/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/nsm/NSMCommunicator.h"
#include "daq/slc/nsm/NSMHandlerException.h"

#include "daq/slc/version/Version.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstdarg>

using namespace Belle2;

NSMCallback::NSMCallback(const int timeout)
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

  addDefaultHandlers();
}


int NSMCallback::reset()
{
  int revision = Callback::reset();

  addDefaultHandlers();

  return revision;
}

int NSMCallback::addDefaultHandlers()
{
  return add(new NSMVHandlerText("version", true, false, DAQ_SLC_VERSION::DAQ_SLC_VERSION), false, true);
}

void NSMCallback::addNode(const NSMNode& node)
{
  std::string name = node.getName();
  if (m_nodes.find(name) == m_nodes.end()) {
    m_nodes.insert(NSMNodeMap::value_type(name, node));
  }
}

void NSMCallback::reply(const NSMMessage& msg)
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
  vsnprintf(ss, sizeof(ss), format, ap);
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
                                                     pri, getCategory(), msg)));
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

bool NSMCallback::perform(NSMCommunicator& com)
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
    if (msg.getNParams() > 2) lmsg.setCategory(msg.getParam(2));
    lmsg.setMessage(msg.getLength() > 0 ? msg.getData() : "");
    logset(lmsg);
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
  }
  return false;
}

void NSMCallback::notify(const NSMVar& var)
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

void NSMCallback::vget(const std::string& nodename, const std::string& vname)
{
  NSMNode node(nodename);
  NSMVar var(vname);
  try {
    NSMVHandler* handler_p = getHandler_p(nodename, vname);
    if (handler_p) {
      NSMVHandler& handler(*handler_p);
      handler.handleGet(var);
      var.setNode(getNode().getName());
      var.setName(vname);
      //var.setId(i + 1);
      var.setDate(Date());
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
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void NSMCallback::vset(NSMCommunicator& com, const NSMVar& var)
{
  try {
    NSMMessage msg(com.getMessage());
    NSMVHandler* handler_p = getHandler_p(var.getNode(), var.getName());
    if (handler_p) {
      NSMVHandler& handler(*handler_p);
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

void NSMCallback::vlistget(NSMCommunicator& com)
{
  std::stringstream ss;
  int i = 0;
  int count = 0;
  for (NSMVHandlerList::iterator it = m_handler.begin();
       it != m_handler.end(); it++) {
    NSMVHandler& handler(*it->second);
    if (handler.getNode().size() == 0) {
      ss << handler.getName() << ":"
         << handler.get().getTypeLabel() << ":"
         << (int)handler.useGet() << ":"
         << (int)handler.useSet() << ":"
         << i << "\n";
      count++;
    }
    i++;
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

void NSMCallback::vlistset(NSMCommunicator& com)
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

void NSMCallback::alloc_open(NSMCommunicator& com)
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

