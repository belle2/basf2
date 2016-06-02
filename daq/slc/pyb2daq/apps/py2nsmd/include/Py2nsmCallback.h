#ifndef _Belle2_Py2nsmCallback_h
#define _Belle2_Py2nsmCallback_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class Py2nsmCallback : public NSMCallback {

  public:
    Py2nsmCallback(const std::string& nodename, int timout = 5);
    virtual ~Py2nsmCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
