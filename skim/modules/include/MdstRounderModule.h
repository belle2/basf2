/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/dataobjects/Particle.h>

#include <Math/Vector4D.h>
#include <TMatrixDSym.h>
#include <TMatrixFSym.h>


namespace Belle2 {

  /**
   * MdstRounder module
   * Rounds all Double32_t members of all ECLClusters and TrackFitResults to precision as if read from mdst
   * Updates all already created particles
   */
  class MdstRounderModule : public Module {

  public:

    /**
     * Constructor.
     */
    MdstRounderModule();

    /**
     * Initialization.
     */
    void initialize() override;

    /**
     * Event handling.
     */
    void event() override;

  private:

    /**
     * Clamps value to range [min, max] and rounds to precision corresponding to nBits number of bits
     */
    double roundToPrecision(Double32_t value, double min, double max, int nBits) const;

    /**
     * Calculates 7x7 error matrix for particle four-momentum and vertex from 6x6 momentum-vertex covariance matrix and energy
     */
    TMatrixFSym getErrMatrixFromCov(const TMatrixDSym& cov6, const ROOT::Math::PxPyPzEVector& vec) const;

    /**
     * Round Double32_t members of all ECLClusters to appropriate precision
     */
    void roundECLClusters() const;

    /**
     * Rounds Double32_t members of all TrackFitResults to appropriate precision
     */
    void roundTrackFitResults() const;

    /**
     * Updates all particles previously created with new rounded values
     */
    void updateParticles();

    /**
     * StoreArray of ECLClusters
     */
    StoreArray<ECLCluster> m_eclclusters;

    /**
     * StoreArray of TrackFitResults
     */
    StoreArray<TrackFitResult> m_trackfitresults;

    /**
     * StoreArray of Particles
     */
    StoreArray<Particle> m_particles;

    /**
     * ClusterUtils to compute four-momentum from ECLClusters
     */
    ClusterUtils m_C;

  };

}