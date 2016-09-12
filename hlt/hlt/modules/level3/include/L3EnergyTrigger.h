/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L3ENERGYTRIGGER_H
#define L3ENERGYTRIGGER_H

#include "hlt/hlt/modules/level3/L3TriggerBase.h"

namespace Belle2 {

  //! The Level3 energy trigger
  class L3EnergyTrigger : public L3TriggerBase {
  public:
    //! Constructor
    L3EnergyTrigger();

    //! Destructor
    virtual ~L3EnergyTrigger() {}

    //! selecct events
    virtual bool select(L3Tag* tag);

    //! returns reference of cut parameters
    double& minEnergy() { return m_minEnergy; }

  private:
    //! cut parameters
    double m_minEnergy;
  };
}

#endif
