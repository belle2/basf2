#ifndef _Belle2_RCSequencer_h
#define _Belle2_RCSequencer_h

#include "daq/slc/apps/runcontrold/RunControlMessage.h"

#include <daq/slc/system/Cond.h>

#include <list>

namespace Belle2 {

  class RCMaster;

  class RCSequencer {

  public:
    typedef std::list<RCSequencer*> RCSequencerList;
    static void notify(bool killed = false);

  private:
    static RCSequencerList __seq_l;
    static Mutex __mutex;
    static Cond __cond;
    static bool __killed;

  public:
    RCSequencer(RCMaster* master,
                RunControlMessage msg,
                bool synchronized = true);
    ~RCSequencer();

  public:
    void run() throw();

  private:
    RCMaster* _master;
    RunControlMessage _msg;
    bool _synchronized;

  };

}

#endif
