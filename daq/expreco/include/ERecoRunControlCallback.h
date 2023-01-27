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
    void initialize(const DBObject& obj) override;
    bool perform(NSMCommunicator& com) override;

  private:
    ERecoMasterCallback* m_callback;

  };

}

#endif
