/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RFRunControlCallback_hh
#define _Belle2_RFRunControlCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class RFMasterCallback;

  class RFRunControlCallback : public RCCallback {

  public:
    RFRunControlCallback(RFMasterCallback* callback);
    virtual ~RFRunControlCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual bool perform(NSMCommunicator& com) throw();

  private:
    RFMasterCallback* m_callback;

  };

}

#endif
