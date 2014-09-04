#ifndef _Belle2_RFMasterCallback_hh
#define _Belle2_RFMasterCallback_hh

#include "daq/rfarm/manager/RFConf.h"

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/runcontrol/RCState.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RFMaster;
  class RFRunControlCallback;

  class RFMasterCallback : public NSMCallback {

    typedef std::map<std::string, NSMNode> NSMNodeList;
    typedef NSMNodeList::iterator NSMNodeIterator;
    typedef NSMNodeList::reverse_iterator NSMNodeRIterator;

  public:
    RFMasterCallback(const NSMNode& node,
                     const NSMData& data,
                     RFMaster* master,
                     const char* confile);
    virtual ~RFMasterCallback() throw();

  public:
    virtual bool perform(const NSMMessage& msg) throw();
    virtual void init() throw();
    virtual void timeout() throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool configure() throw();
    virtual bool unconfigure() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool pause() throw();
    virtual bool resume() throw();
    virtual bool restart() throw();
    virtual bool status() throw();

  public:
    void setCallback(RFRunControlCallback* callback) {
      m_callback = callback;
    }
    void reply(bool result);
    void setState(const RCState& state);

  private:
    NSMData m_data;
    RFMaster* m_master;
    std::vector<NSMData> m_data_v;
    RFRunControlCallback* m_callback;
    RFConf m_conf;
    int m_st_conf;
    int m_st_unconf;
    std::vector<std::string> m_name_v;
    NSMNodeList m_nodes;

  };

}

#endif
