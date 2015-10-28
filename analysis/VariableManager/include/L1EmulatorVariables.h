/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/ECLCluster.h>

namespace Belle2 {

  namespace Variable {

    /**
     * The vetos used in LE are all defined based on the
     * the good charged tracks and the ECL clusters passing the selection
     * criteria
     */

    /**
    * return ecl Bhabha veto result
    */
    double eclBhabha(const Particle*);

    /**
    * return Bhabha veto result
    */
    double BhabhaVeto(const Particle*);

    /**
    * return Bhabha veto result with single track
    */
    double SBhabhaVeto(const Particle*);

    /**
    * return gg veto result
    */
    double ggVeto(const Particle*);


  }

}
