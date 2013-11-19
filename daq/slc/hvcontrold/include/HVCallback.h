#ifndef _Belle2_HVCallback_hh
#define _Belle2_HVCallback_hh

#include "HVCrateInfo.h"

#include "nsm/NSMCallback.h"

#include "base/Command.h"
#include "base/NSMNode.h"

namespace Belle2 {

  class HVCallback : public NSMCallback {

  public:
    HVCallback(NSMNode* node) throw();
    virtual ~HVCallback() throw() {}

  public:
    virtual bool load() throw() { return true; }
    virtual bool switchOn() throw() { return true; }
    virtual bool switchOff() throw() { return true; }
    void download() throw();

  protected:
    virtual bool perform(NSMMessage& msg)
    throw(NSMHandlerException);

  public:
    HVCrateInfo* getCrate() { return m_crate; }

  private:
    HVCrateInfo* m_crate;

  };

};

#endif
