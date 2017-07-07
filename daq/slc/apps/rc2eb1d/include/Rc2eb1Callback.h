#ifndef _Belle2_Rc2eb1Callback_h
#define _Belle2_Rc2eb1Callback_h

#include "daq/slc/nsm/NSMCallback.h"

#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  class Rc2eb1Callback : public NSMCallback {

  public:
    typedef struct node_hosts {
      std::string name;
      std::vector<std::string> hosts;
    } node_hosts_t;

  public:
    Rc2eb1Callback(const std::string& nodename, ConfigFile& conf,
                   int timout = 5);
    virtual ~Rc2eb1Callback() throw();

  public:
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  private:
    NSMNode m_rcnode;
    NSMNode m_eb1;
    std::vector<node_hosts_t> m_nodes;

  };

}

#endif
