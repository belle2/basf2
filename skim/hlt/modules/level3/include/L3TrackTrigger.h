/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3TRACKTRIGGER_H
#define L3TRACKTRIGGER_H

#include "skim/hlt/modules/level3/L3TriggerBase.h"

namespace Belle2 {
  class L3TrackTrigger : public L3TriggerBase {
  public:
    L3TrackTrigger() {SetName("TrackTrigger");}
    virtual ~L3TrackTrigger() {}

    //! selecct events
    virtual bool select(L3Tag* tag);

    //! returns reference of cut parameters
    double& drCut() { return m_drCut; }
    double& dzCut() { return m_dzCut; }
    double& ptCut() { return m_ptCut; }
    int& minNGoodTrks() { return m_minNGoodTrks; }

  private:
    //! cut parameters
    double m_drCut;
    double m_dzCut;
    double m_ptCut;
    int m_minNGoodTrks;
  };
}

#endif
