#ifndef _Belle2_RunControlMasterCallback_h
#define _Belle2_RunControlMasterCallback_h

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/runcontrol/RunControlCallback.h"

#include "daq/slc/database/RunNumberInfoTable.h"
#include "daq/slc/runcontrol/RunSetting.h"
#include "daq/slc/runcontrol/RunSummary.h"

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMNode.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RunControlMasterCallback : public RCCallback {

    typedef std::vector<NSMNode> NSMNodeList;
    typedef NSMNodeList::iterator NSMNodeIterator;
    typedef std::map<std::string, NSMNode> NSMNodeMap;
    typedef std::vector<NSMData> NSMDataList;

  public:
    RunControlMasterCallback(const NSMNode& node,
                             RunControlCallback* callback);
    virtual ~RunControlMasterCallback() throw() {}

  public:
    virtual void init() throw();
    virtual void timeout() throw();
    virtual bool perform(const NSMMessage& msg) throw();
    virtual void update() throw();

  protected:
    NSMData m_data;
    RunControlCallback* m_callback;

  };

}

#endif

