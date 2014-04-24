#ifndef _Belle2_VMEControllerCallback_hh
#define _Belle2_VMEControllerCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class VMEControllerCallback : public RCCallback {

  public:
    VMEControllerCallback(const NSMNode& node);
    virtual ~VMEControllerCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw();

  };

}

#endif
