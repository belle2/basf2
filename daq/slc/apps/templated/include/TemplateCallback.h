#ifndef _Belle2_TemplateCallback_hh
#define _Belle2_TemplateCallback_hh

#include <daq/slc/nsm/RCCallback.h>

namespace Belle2 {

  class TemplateCallback : public RCCallback {

  public:
    TemplateCallback(NSMNode* node = NULL);
    virtual ~TemplateCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw();
    virtual bool trigft() throw();

  };

}

#endif
