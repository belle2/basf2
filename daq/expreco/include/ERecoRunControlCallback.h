/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ERecoRunControlCallback_hh
#define _Belle2_ERecoRunControlCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>

namespace Belle2 {

  class ERecoMasterCallback;

  class ERecoRunControlCallback : public RCCallback {

  public:
    ERecoRunControlCallback(ERecoMasterCallback* callback);
    virtual ~ERecoRunControlCallback() {}

  public:
    virtual void initialize(const DBObject& obj);
    virtual bool perform(NSMCommunicator& com);

  private:
    ERecoMasterCallback* m_callback;

  };

}

#endif
