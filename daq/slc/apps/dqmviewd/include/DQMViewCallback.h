#ifndef _Belle2_DQMViewCallback_h
#define _Belle2_DQMViewCallback_h

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/base/ConfigFile.h>

#include <daq/slc/apps/dqmviewd/HistSender.h>

#include <vector>

namespace Belle2 {

  class DQMViewCallback : public NSMCallback {

  public:
    DQMViewCallback(const NSMNode& node, ConfigFile& config);
    virtual ~DQMViewCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  public:
    std::vector<HistSender>& getSenders() { return m_senders; }
    const std::vector<HistSender>& getSenders() const { return m_senders; }
    void addSender(const HistSender& sender) { m_senders.push_back(sender); }

  private:
    std::vector<HistSender> m_senders;

  };

}

#endif
