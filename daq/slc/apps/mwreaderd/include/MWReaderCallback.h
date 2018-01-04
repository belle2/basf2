#ifndef _Belle2_MWReaderCallback_h
#define _Belle2_MWReaderCallback_h

#include "daq/slc/apps/mwreaderd/MWReaderImp.h"

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class MWReaderCallback : public NSMCallback {

  public:
    MWReaderCallback(const std::string& nodename, int timout = 2);
    virtual ~MWReaderCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  private:
    MWReader m_reader;

  };

}

#endif
