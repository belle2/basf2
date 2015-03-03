#include "daq/slc/nsm/AbstractNSMCallback.h"
#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"
#include "daq/slc/system/Time.h"

#include "daq/slc/database/DBObject.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <signal.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace Belle2;

AbstractNSMCallback::AbstractNSMCallback(int timeout) throw()
{
  m_timeout = timeout;
}

NSMCommunicator& AbstractNSMCallback::wait(const NSMNode& node,
                                           const NSMCommand& cmd,
                                           int timeout) throw(IOException)
{
  try {
    double t0 = Time().get();
    while (true) {
      NSMCommunicator& com(NSMCommunicator::select(timeout));
      NSMMessage msg = com.getMessage();
      const std::string reqname = msg.getRequestName();
      if ((cmd == NSMCommand::UNKNOWN || cmd == reqname) &&
          (node.getName().size() == 0 || msg.getNodeName() == node.getName())) {
        return com;
      }
      com.getCallback().perform(com);
      if (timeout > 0 && Time().get() - t0 >= timeout)
        break;
    }
  } catch (const TimeoutException& e) {}
  throw (TimeoutException("timeout : waiting for %s from %s",
                          cmd.getLabel(), node.getName().c_str()));
}

void AbstractNSMCallback::readVar(const NSMMessage& msg, NSMVar& var)
{
  const int* pars = msg.getParams();
  const char* name = msg.getData();
  const char* value = (msg.getData() + pars[2] + 1);
  var = NSMVar(name, (NSMVar::Type)pars[0], pars[1], value);
  if (msg.getNParams() > 3) {
    var.setId(pars[3]);
    var.setRevision(pars[4]);
    var.setNodeId(pars[5]);
  }
}

bool AbstractNSMCallback::get(const NSMNode& node, NSMVar& var,
                              int timeout) throw(IOException)
{
  if (node.getName().size() > 0) {
    const std::string name = var.getName();
    NSMCommunicator::send(NSMMessage(node, NSMCommand::VGET, name));
    double t0 = Time().get();
    while (true) {
      try {
        NSMCommunicator& com(wait(node, NSMCommand::VSET, timeout));
        NSMMessage msg = com.getMessage();
        NSMCommand cmd(msg.getRequestName());
        if (cmd == NSMCommand::VSET) {
          if (var.getName() == msg.getData()) {
            readVar(msg, var);
            NSMVHandler* handler = NSMVHandler::create(var);
            if (handler) {
              handler->setNode(node.getName());
              bool ret = handler->handleSet(var);
              if (ret) {
                add(handler);
              } else {
                delete handler;
              }
              return ret;
            }
          }
          com.getCallback().perform(com);
        }
      } catch (const IOException& e) {}
      if (timeout > 0 && Time().get() - t0 >= timeout) {
        throw (TimeoutException("timeout : waiting for VSET:s from %s",
                                var.getName().c_str(), node.getName().c_str()));
      }
    }
  }
  return false;
}

bool AbstractNSMCallback::set(const NSMNode& node, const NSMVar& var,
                              int timeout) throw(IOException)
{
  if (node.getName().size() > 0) {
    NSMCommunicator::send(NSMMessage(node, var));
    double t0 = Time().get();
    while (true) {
      try {
        NSMCommunicator& com(wait(node, NSMCommand::VREPLY, timeout));
        NSMMessage msg(com.getMessage());
        NSMCommand cmd(msg.getRequestName());
        if (cmd == NSMCommand::VREPLY && var.getName() == msg.getData()) {
          return msg.getParam(0) > 0;
        }
        com.getCallback().perform(com);
      } catch (const TimeoutException& e) {}
      if (timeout > 0 && Time().get() - t0 >= timeout) {
        throw (TimeoutException("timeout : waiting for VSET:%s from %s",
                                var.getName().c_str(), node.getName().c_str()));
      }
    }
  }
  return false;
}

bool AbstractNSMCallback::get(const NSMNode& node, const std::string& name,
                              int& val, int timeout) throw(IOException)
{
  return get_t(node, name, val, timeout, NSMVar::INT, 0);
}

bool AbstractNSMCallback::get(const NSMNode& node, const std::string& name,
                              float& val, int timeout) throw(IOException)
{
  return get_t(node, name, val, timeout, NSMVar::FLOAT, 0);
}

bool AbstractNSMCallback::get(const NSMNode& node, const std::string& name,
                              std::string& val, int timeout) throw(IOException)
{
  return get_t(node, name, val, timeout, NSMVar::TEXT, 1);
}

bool AbstractNSMCallback::get(const NSMNode& node, const std::string& name,
                              std::vector<int>& val, int timeout) throw(IOException)
{
  return get_t(node, name, val, timeout, NSMVar::INT, 1);
}

bool AbstractNSMCallback::get(const NSMNode& node, const std::string& name,
                              std::vector<float>& val, int timeout) throw(IOException)
{
  return get_t(node, name, val, timeout, NSMVar::FLOAT, 1);
}

bool AbstractNSMCallback::set(const NSMNode& node, const std::string& name,
                              int val, int timeout) throw(IOException)
{
  return set(node, NSMVar(name, val), timeout);
}

bool AbstractNSMCallback::set(const NSMNode& node, const std::string& name,
                              float val, int timeout) throw(IOException)
{
  return set(node, NSMVar(name, val), timeout);
}

bool AbstractNSMCallback::set(const NSMNode& node, const std::string& name,
                              const std::string& val, int timeout) throw(IOException)
{
  return set(node, NSMVar(name, val), timeout);
}

bool AbstractNSMCallback::set(const NSMNode& node, const std::string& name,
                              const std::vector<int>& val, int timeout) throw(IOException)
{
  return set(node, NSMVar(name, val), timeout);
}

bool AbstractNSMCallback::set(const NSMNode& node, const std::string& name,
                              const std::vector<float>& val, int timeout) throw(IOException)
{

  return set(node, NSMVar(name, val), timeout);
}

bool AbstractNSMCallback::get(DBObject& obj)
{
  DBObject::NameValueList list;
  obj.search(list);
  for (DBObject::NameValueList::iterator it = list.begin();
       it != list.end(); it++) {
    const std::string& name(it->name);
    int vi = 0;
    float vf = 0;
    std::string vs;
    switch (it->type) {
      case DBField::BOOL:
        if (get(name, vi)) *((bool*)it->buf) = (bool)vi;
        break;
      case DBField::CHAR:
        if (get(name, vi)) *((char*)it->buf) = (char)vi;
        break;
      case DBField::SHORT:
        if (get(name, vi)) *((short*)it->buf) = (short)vi;
        break;
      case DBField::INT:
        if (get(name, vi)) *((int*)it->buf) = (int)vi;
        break;
      case DBField::LONG:
        if (get(name, vi)) *((long long*)it->buf) = vi;
        break;
      case DBField::FLOAT:
        if (get(name, vf)) *((float*)it->buf) = vf;
        break;
      case DBField::DOUBLE:
        if (get(name, vf)) *((double*)it->buf) = vf;
        break;
      case DBField::TEXT:
        if (get(name, vs)) *((std::string*)it->buf) = vs;
        break;
      default:
        break;
    }
  }
  return true;
}

