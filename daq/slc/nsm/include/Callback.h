#ifndef _Belle2_Callback_hh
#define _Belle2_Callback_hh

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMVar.h"
#include "daq/slc/nsm/NSMNode.h"
#include "daq/slc/nsm/NSMVHandler.h"

#include "daq/slc/database/DBObject.h"

#include "daq/slc/system/LogFile.h"

#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <stdexcept>

namespace Belle2 {

  class Callback {

    friend class AbstractNSMCallback;
    friend class NSMCallback;

  public:
    Callback() throw() : m_revision(1) {}
    virtual ~Callback() throw() {}

  public:
    int reset();
    int add(const DBObject& obj);
    int add(NSMVHandler* handler);
    void remove(const std::string& node, const std::string& name);
    void remove(const std::string& name) { remove("", name); }
    void remove(const DBObject& obj);
    NSMVHandlerList& getHandlers() { return m_handler; }
    NSMVHandler& getHandler(const std::string& name) throw(std::out_of_range) {
      return getHandler("", name);
    }
    NSMVHandler& getHandler(const std::string& node, const std::string& name)
    throw(std::out_of_range);
    void setUseGet(const std::string& name, bool use) throw(std::out_of_range) {
      getHandler(name).setUseGet(use);
    }
    void setUseSet(const std::string& name, bool use) throw(std::out_of_range) {
      getHandler(name).setUseSet(use);
    }

  public:
    void setRevision(int revision) throw () { m_revision = revision; }
    void setObject(const DBObject& obj) throw() { m_obj = obj; }
    int getRevision() const throw () { return m_revision; }
    DBObject& getObject() throw() { return m_obj; }
    const DBObject& getObject() const throw() { return m_obj; }

  private:
    NSMVHandler* getHandler_p(const std::string& node, const std::string& name);

  private:
    int m_revision;
    DBObject m_obj;
    NSMVHandlerList m_handler;

  };

};

#endif
