/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/Callback.h"

#include "daq/slc/base/StringUtil.h"

using namespace Belle2;

int Callback::reset()
{
  for (NSMVHandlerList::iterator it = m_handler.begin();
       it != m_handler.end(); it++) {
    if (it->second != NULL) delete it->second;
  }
  m_handler = NSMVHandlerList();
  m_hnames = StringList();
  return ++m_revision;
}

int Callback::add(NSMVHandler* handler, bool overwrite, bool isdump)
{
  if (handler) {
    std::string hname = handler->getNode() + "@" + handler->getName();
    if (m_handler.find(hname) != m_handler.end()) {
      NSMVHandler* handler_old = m_handler.find(hname)->second;
      if (handler->useSet() && overwrite) {
        handler->set(handler_old->get());
      }
      delete handler_old;
      m_handler.erase(hname);
    }
    handler->setDumped(isdump);
    m_handler.insert(std::pair<std::string, NSMVHandler*>(hname, handler));
    m_hnames.push_back(hname);
    return m_handler.size();
  }
  return 0;
}

void Callback::remove(const std::string& node, const std::string& name)
{
  std::string hname = node + "@" + node;
  if (m_handler.find(hname) != m_handler.end()) {
    NSMVHandler* handler_old = m_handler.find(hname)->second;
    delete handler_old;
    m_handler.erase(hname);
    return;
  }
}

NSMVHandler& Callback::getHandler(const std::string& node,
                                  const std::string& name)
{
  NSMVHandler* p = getHandler_p(node, name);
  if (p != NULL) return *p;
  throw (std::out_of_range(StringUtil::form("no handler for %s:%s",
                                            node.c_str(), name.c_str())));
}

NSMVHandler* Callback::getHandler_p(const std::string& node, const std::string& name)
{
  std::string hname = node + "@" + name;
  if (m_handler.find(hname) != m_handler.end()) {
    return m_handler.find(hname)->second;
  }
  hname = "@" + name;
  if (m_handler.find(hname) != m_handler.end()) {
    return m_handler.find(hname)->second;
  }
  return NULL;
}

void Callback::remove(const DBObject& obj)
{
  DBObject::NameValueList list;
  obj.search(list, "", true);
  for (DBObject::NameValueList::iterator it = list.begin();
       it != list.end(); it++) {
    const std::string& name(it->name);
    if (name.size() == 0 || name.at(0) == '$') continue;
    switch (it->type) {
      case DBField::OBJECT:
        break;
      default:
        remove(name);
        break;
    }
  }
}
