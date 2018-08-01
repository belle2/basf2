/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Yu Hu                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <mdst/dataobjects/MCParticle.h>
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {


    /**
     * return Chi2 of four momentum-constraint kinematical fit of KFit
     */
    double Chi2(const Particle* part);

    /**
     * return Prob of four momentum-constraint kinematical fit of KFit
     */
    double Prob(const Particle* part);


  }
} // Belle2 namespace
