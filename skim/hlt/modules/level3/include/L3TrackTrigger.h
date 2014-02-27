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

  //! The Level3 track trigger
  class L3TrackTrigger : public L3TriggerBase {
  public:
    //! Constructor
    L3TrackTrigger();

    //! Destructor
    virtual ~L3TrackTrigger() {}

    //! selecct events
    virtual bool select(L3Tag* tag);

    //! returns cut parameter (drho)
    double& drCut() { return m_drCut; }
    //! returns cut parameter (dz)
    double& dzCut() { return m_dzCut; }
    //! returns cut parameter (pt)
    double& ptCut() { return m_ptCut; }
    //! returns cut parameter (minimum number of good tracks)
    int& minNGoodTrks() { return m_minNGoodTrks; }

  private:
    //! cut parameter (drho)
    double m_drCut;
    //! cut parameter (dz)
    double m_dzCut;
    //! cut parameter (pt)
    double m_ptCut;
    //! cut parameter (minimum number of good tracks)
    int m_minNGoodTrks;
  };
}

#endif
