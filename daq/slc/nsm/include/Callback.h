/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Callback_hh
#define _Belle2_Callback_hh

#include "daq/slc/nsm/NSMVar.h"
#include "daq/slc/nsm/NSMVHandler.h"

#include "daq/slc/database/DBObject.h"

#include <cstring>

namespace Belle2 {

  class Callback {

    friend class AbstractNSMCallback;
    friend class NSMCallback;

  public:
    Callback() : m_revision(1) {}
    virtual ~Callback() {}

  public:
    virtual void notify(const NSMVar& var) = 0;
    int reset();
    int add(NSMVHandler* handler, bool overwrite = true, bool isdump = true);
    void remove(const std::string& node, const std::string& name);
    void remove(const std::string& name) { remove("", name); }
    void remove(const DBObject& obj);
    StringList& getHandlerNames() { return m_hnames; }
    NSMVHandlerList& getHandlers() { return m_handler; }
    NSMVHandler& getHandler(const std::string& name)
    {
      return getHandler("", name);
    }
    NSMVHandler& getHandler(const std::string& node, const std::string& name)
    ;
    void setUseGet(const std::string& name, bool use)
    {
      getHandler(name).setUseGet(use);
    }
    void setUseSet(const std::string& name, bool use)
    {
      getHandler(name).setUseSet(use);
    }

  public:
    void setRevision(int revision) { m_revision = revision; }
    int getRevision() const { return m_revision; }

  private:
    NSMVHandler* getHandler_p(const std::string& node, const std::string& name);

  private:
    int m_revision;
    NSMVHandlerList m_handler;
    StringList m_hnames;

  };

};

#endif
