/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


    /** Returns first_x from mdst_trk_fit */
    double BelleFirstCDCHitX(const Particle* particle);
    /** Returns first_y from mdst_trk_fit */
    double BelleFirstCDCHitY(const Particle* particle);
    /** Returns first_z from mdst_trk_fit */
    double BelleFirstCDCHitZ(const Particle* particle);

    /** Returns last_x from mdst_trk_fit */
    double BelleLastCDCHitX(const Particle* particle);
    /** Returns last_y from mdst_trk_fit */
    double BelleLastCDCHitY(const Particle* particle);
    /** Returns last_z from mdst_trk_fit */
    double BelleLastCDCHitZ(const Particle* particle);

    /** Returns significance of invariant mass using daughters' covariance matrices */
    double BellePi0InvariantMassSignificance(const Particle* particle);
  }
}


