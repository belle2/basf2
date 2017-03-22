#include "daq/slc/nsm/Callback.h"
#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/StringUtil.h"

#include <algorithm>

using namespace Belle2;

int Callback::reset()
{
  for (NSMVHandlerList::iterator it = m_handler.begin();
       it != m_handler.end(); it++) {
    if (*it != NULL) delete *it;
  }
  m_handler = NSMVHandlerList();
  return ++m_revision;
}

int Callback::add(NSMVHandler* handler)
{
  if (handler) {
    for (size_t i = 0; i < m_handler.size(); i++) {
      if (handler->getNode() == m_handler[i]->getNode() &&
          handler->getName() == m_handler[i]->getName()) {
        delete m_handler[i];
        m_handler[i] = handler;
        notify(m_handler[i]->get());
        return i + 1;
      }
    }
    //if (handler->getNode().size() == 0)
    //  LogFile::debug("added %s", handler->getName().c_str());
    m_handler.push_back(handler);
    return m_handler.size();
  }
  return 0;
}

void Callback::remove(const std::string& node, const std::string& name)
{
  for (NSMVHandlerList::iterator it = m_handler.begin();
       it != m_handler.end(); it++) {
    if (node == (*it)->getNode() &&
        name == (*it)->getName()) {
      if (*it != NULL) delete *it;
      m_handler.erase(it);
      return;
    }
  }
}

NSMVHandler& Callback::getHandler(const std::string& node,
                                  const std::string& name) throw(std::out_of_range)
{
  NSMVHandler* p = getHandler_p(node, name);
  if (p != NULL) return *p;
  throw (std::out_of_range(StringUtil::form("no handler for %s:%s",
                                            node.c_str(), name.c_str())));
}

NSMVHandler* Callback::getHandler_p(const std::string& node, const std::string& name)
{
  for (size_t i = 0; i < m_handler.size(); i++) {
    if (node == m_handler[i]->getNode() &&
        name == m_handler[i]->getName()) {
      return m_handler[i];
    }
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
