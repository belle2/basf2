/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Minakshi Nayak, Sam Cunliffe                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    /** Returns true if the particle candidate passes the Belle 'good Kshort' algorithm */
    double goodBelleKshort(const Particle* KS);

    /** return goodLambda using Belle goodLambda selection rules */
    double goodBelleLambda(const Particle* Lambda);

    /** Returns true if the cluster with given attributes passes the Belle 'good gamma' criteria. */
    bool isGoodBelleGamma(int region, double energy);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 100 MeV
     * Barrel   : E >  50 MeV
     * Backward : E > 150 MeV
     */
    double goodBelleGamma(const Particle* particle);

  }
}


