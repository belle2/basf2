/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// BEAMPARAMETERS AND DATABASE
#include <framework/dbobjects/BeamParameters.h>
#include <framework/database/DBObjPtr.h>

// MDST
#include <mdst/dataobjects/ECLCluster.h>

// ROOT
#include <TLorentzVector.h>
#include <TMatrixD.h>

namespace Belle2 {

  /**
   * Class to provide momentum-related information from ECLClusters.
   * This requires the prior knowledge of the particle type (e.g., photon),
   * and a vertex hypothesis. The default vertex hypothesis is taken from the
   * beamparameters.
   */
  class ClusterUtils {

  public:

    /**
     * Constructor
     */
    ClusterUtils();

    /**
     * Returns four momentum vector
     * @return const four momentum vector
     */
    const TLorentzVector Get4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo, double mass = 0.);

    /**
     * Returns four momentum vector
     * @return const four momentum vector
     */
    const TLorentzVector Get4MomentumFromCluster(const ECLCluster* cluster, const TVector3& vertex, ECLCluster::EHypothesisBit hypo,
                                                 double mass = 0.);

    /**
     * Returns 4x4 covariance matrix (px, py, pz, E)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 4x4 covariance matrix (px, py, pz, E)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster, const TVector3& vertex,
                                                        const TMatrixDSym& covmatvertex, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 7x7 covariance matrix (px, py, pz, E, x, y, z)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 7x7 covariance matrix (px, py, pz, E, x, y, z)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster, const TVector3& vertex,
                                                        const TMatrixDSym& covmatvertex, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns default IP position from beam parameters
     * @return const TVector3
     */
    const TVector3 GetIPPosition();

    /**
     * Returns default IP position covariance matrix from beam parameters
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetIPPositionCovarianceMatrix();

  private:

    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */
  };

} // Belle2 namespace
