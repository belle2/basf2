/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {

  namespace Variable {

    /**
     * The variables used in HLT are all defined based on the
     * the good charged tracks and the ECL clusters passing the selection
     * criteria
     */


    /**
     * return the number of the charged tracks
     */
    double nTracksHLT(const Particle*);

    /**
     * return the number of the ECL clusters
     */
    double nECLClustersHLT(const Particle*);

    /**
     * return the sum energy of the  ECL clusters
     */
    double ECLEnergyHLT(const Particle*);

    /**
     * return the visible energy defined as the sum
     * of the track momenta and the ECL cluster energies
     */
    double VisibleEnergyHLT(const Particle*);

    /**
     * return the sum of the absolute Pz of the total visible energy
     */
    double VisiblePzHLT(const Particle*);

    /**
     * return the angle between the two tracks
     * with the first and second largest momentum
     */
    double AngleTTHLT(const Particle*);

    /**
     * return the maximum angle between the two tracks
     */
    double MaxAngleTTHLT(const Particle*);

    /**
     * return the largest momentum for the charged tracks
     */
    double TrackP1HLT(const Particle*);

    /**
     * return the second largest momentum for the charged tracks
     */
    double TrackP2HLT(const Particle*);

    /**
     * return the angle between the largest and second energetic ECL clusters
     */
    double AngleGGHLT(const Particle*);

    /**
     * return the largest energy of the ECL clusters
     */
    double ECLClusterE1HLT(const Particle*);

    /**
     * return the second largest energy of the ECL clusters
     */
    double ECLClusterE2HLT(const Particle*);

    /**
     * return the energy of the ECL clusters
     */
    double ECLClusterEHLT(const Particle*);

    /**
     * return the polar angle of the ECL clusters
     */
    double ECLClusterThetaHLT(const Particle*);

    /**
     * return the timing information of the ECL clusters
     */
    double ECLClusterTimingHLT(const Particle*);

    /**
     * return the number of KLM layters with hits
     */
    double nKLMClustersHLT(const Particle*);

  }

}
