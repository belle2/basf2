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
    virtual void notify(const NSMVar& var) throw() = 0;
    int reset();
    int add(NSMVHandler* handler);
    void remove(const std::string& node, const std::string& name);
    void remove(const std::string& name) { remove("", name); }
    void remove(const DBObject& obj);
    NSMVHandlerList& getHandlers() { return m_handler; }
    NSMVHandler& getHandler(const std::string& name) throw(std::out_of_range)
    {
      return getHandler("", name);
    }
    NSMVHandler& getHandler(const std::string& node, const std::string& name)
    throw(std::out_of_range);
    void setUseGet(const std::string& name, bool use) throw(std::out_of_range)
    {
      getHandler(name).setUseGet(use);
    }
    void setUseSet(const std::string& name, bool use) throw(std::out_of_range)
    {
      getHandler(name).setUseSet(use);
    }

  public:
    void setRevision(int revision) throw () { m_revision = revision; }
    int getRevision() const throw () { return m_revision; }

  private:
    NSMVHandler* getHandler_p(const std::string& node, const std::string& name);

  private:
    int m_revision;
    NSMVHandlerList m_handler;

  };

};

#endif
