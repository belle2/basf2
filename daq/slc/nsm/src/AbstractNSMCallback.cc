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

int AbstractNSMCallback::addDB(const DBObject& obj)
{
  DBObject::NameValueList list;
  obj.search(list, "", true);
  int id = 0;
  for (DBObject::NameValueList::iterator it = list.begin();
       it != list.end(); it++) {
    const std::string& name(it->name);
    if (name.size() == 0 || name.at(0) == '$') continue;
    switch (it->type) {
      case DBField::BOOL:
        id = Callback::add(new NSMVHandlerInt("", name, true, true, (int) * ((bool*)it->buf)));
        set(name, (int) * ((bool*)it->buf));
        break;
      case DBField::CHAR:
        id = Callback::add(new NSMVHandlerInt("", name, true, true, (int) * ((char*)it->buf)));
        set(name, (int) * ((char*)it->buf));
        break;
      case DBField::SHORT:
        id = Callback::add(new NSMVHandlerInt("", name, true, true, (int) * ((short*)it->buf)));
        set(name, (int) * ((short*)it->buf));
        break;
      case DBField::INT:
        id = Callback::add(new NSMVHandlerInt("", name, true, true, *((int*)it->buf)));
        set(name, (int) * ((int*)it->buf));
        break;
      case DBField::LONG:
        id = Callback::add(new NSMVHandlerInt("", name, true, true, (int) * ((long long*)it->buf)));
        set(name, (int) * ((long long*)it->buf));
        break;
      case DBField::FLOAT:
        id = Callback::add(new NSMVHandlerFloat("", name, true, true, *((float*)it->buf)));
        set(name, * ((float*)it->buf));
        break;
      case DBField::DOUBLE:
        id = Callback::add(new NSMVHandlerFloat("", name, true, true, (float) * ((double*)it->buf)));
        set(name, (float) * ((double*)it->buf));
        break;
      case DBField::TEXT:
        id = Callback::add(new NSMVHandlerText("", name, true, true, *((std::string*)it->buf)));
        set(name, *((std::string*)it->buf));
        break;
      default:
        break;
    }
  }
  return id;
}

NSMCommunicator& AbstractNSMCallback::wait(const NSMNode& node,
                                           const NSMCommand& cmd,
                                           double timeout) throw(IOException)
{
  double t0 = Time().get();
  double t = t0;
  while (true) {
    double t1 = (timeout - (t - t0) > 0 ? timeout - (t - t0) : 0);
    NSMCommunicator& com(NSMCommunicator::select(t1));
    NSMMessage msg = com.getMessage();
    const std::string reqname = msg.getRequestName();
    if ((cmd == NSMCommand::UNKNOWN || cmd == reqname) &&
        (node.getName().size() == 0 || msg.getNodeName() == node.getName())) {
      return com;
    }
    com.pushQueue(msg);
    t = Time().get();
  }
}

int AbstractNSMCallback::wait(double timeout) throw(IOException)
{
  double t0 = Time().get();
  double t = t0;
  int count = 0;
  try {
    while (true) {
      double t1 = (timeout - (t - t0) > 0 ? timeout - (t - t0) : 0);
      if (t1 == 0) break;
      NSMCommunicator& com(NSMCommunicator::select(t1));
      NSMMessage msg = com.getMessage();
      NSMCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::VGET || cmd == NSMCommand::VLISTGET) {
        perform(com);
      } else {
        com.pushQueue(msg);
      }
      t = Time().get();
    }
  } catch (const TimeoutException& e) {
  }
  return count;
}

bool AbstractNSMCallback::try_wait() throw()
{
  try {
    perform(wait(NSMNode(), NSMCommand::UNKNOWN, 0));
  } catch (const std::exception& e) {
    return false;
  }
  return true;
}

void AbstractNSMCallback::readVar(const NSMMessage& msg, NSMVar& var)
{
  const int* pars = msg.getParams();
  const char* node = msg.getData();
  const char* name = (msg.getData() + pars[2] + 1);
  const char* value = (msg.getData() + pars[2] + 1 + pars[3] + 1);
  var = NSMVar(name, (NSMVar::Type)pars[0], pars[1], value);
  var.setNode(node);
  var.setId(pars[3]);
  var.setRevision(pars[4]);
}

bool AbstractNSMCallback::get(const NSMNode& node, NSMVHandler* handler,
                              int timeout) throw(IOException)
{
  if (handler && node.getName().size() > 0) {
    NSMVar var(handler->get());
    add(handler);
    const std::string name = handler->getName();//var.getName();
    NSMCommunicator::send(NSMMessage(node, NSMCommand::VGET, name));
    double t0 = Time().get();
    double t = t0;
    double tout = timeout;
    while (true) {
      double t1 = (tout - (t - t0) > 0 ? tout - (t - t0) : 0);
      NSMCommunicator& com(wait(NSMNode(), NSMCommand::UNKNOWN, t1));
      NSMMessage msg = com.getMessage();
      NSMCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::VSET) {
        if (node.getName() == msg.getData() &&
            var.getName() == (msg.getData() + msg.getParam(2) + 1)) {
          readVar(msg, var);
          handler->setNode(node.getName());
          return handler->handleSet(var);
        }
      }
      com.pushQueue(msg);
      t = Time().get();
    }
  }
  return false;
}

bool AbstractNSMCallback::get(const NSMNode& node, NSMVar& var,
                              int timeout) throw(IOException)
{
  if (node.getName().size() > 0) {
    const std::string name = var.getName();
    NSMCommunicator::send(NSMMessage(node, NSMCommand::VGET, name));
    double t0 = Time().get();
    double t = t0;
    double tout = timeout;
    while (true) {
      double t1 = (tout - (t - t0) > 0 ? tout - (t - t0) : 0);
      NSMCommunicator& com(wait(node, NSMCommand::VSET, t1));
      NSMMessage msg = com.getMessage();
      NSMCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::VSET) {
        if (msg.getLength() > 0 && msg.getData() != NULL &&
            node.getName() == msg.getData() &&
            var.getName() == (msg.getData() + msg.getParam(2) + 1)) {
          readVar(msg, var);
          NSMVHandler* handler = getHandler_p(node.getName(), var.getName());
          if (!handler) {
            handler = NSMVHandler::create(var);
            if (handler) add(handler);
          }
          if (handler) {
            handler->setNode(node.getName());
            return handler->handleSet(var);
          }
        }
      }
      com.pushQueue(msg);
      t = Time().get();
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
    double t = t0;
    double tout = timeout;
    while (true) {
      double t1 = (tout - (t - t0) > 0 ? tout - (t - t0) : 0);
      NSMCommunicator& com(wait(node, NSMCommand::VREPLY, t1));
      NSMMessage msg(com.getMessage());
      NSMCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::VREPLY && var.getName() == msg.getData()) {
        return msg.getParam(0) > 0;
      }
      com.pushQueue(msg);
      t = Time().get();
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
    if (name.size() == 0 || name.at(0) == '$') continue;
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

