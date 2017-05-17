#ifndef _Belle2_RunrecordCallback_h
#define _Belle2_RunrecordCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObject.h>

#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  class RunrecordCallback : public NSMCallback {

  public:
    RunrecordCallback(const std::string& nodename,
                      ConfigFile& conf, int timeout);
    virtual ~RunrecordCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& v) throw();

  private:
    void update(const std::string& rcstate);

  private:
    DBObjectList m_objs;
    DBInterface* m_db;
    NSMNode m_rcnode;
    std::string m_rcstate;
    std::vector<NSMVar> m_vars;

  };

}

#endif
