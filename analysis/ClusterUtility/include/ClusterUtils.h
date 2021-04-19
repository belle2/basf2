/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// BEAMSPOT AND DATABASE
#include <mdst/dbobjects/BeamSpot.h>
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
     * Returns cluster four momentum vector
     * @return const cluster four momentum vector
     */
    const TLorentzVector GetCluster4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns four momentum vector
     * @return const four momentum vector
     */
    const TLorentzVector Get4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns four momentum vector
     * @return const four momentum vector
     */
    const TLorentzVector Get4MomentumFromCluster(const ECLCluster* cluster, const TVector3& vertex, ECLCluster::EHypothesisBit hypo);
    /**
     * Returns 4x6 Jacobi matrix (px, py, pz, E)
     * @return const TMatrixD
     */
    const TMatrixD GetJacobiMatrix4x6FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 4x6 Jacobi matrix (px, py, pz, E)
     * @return const TMatrixD
     */
    const TMatrixD GetJacobiMatrix4x6FromCluster(const ECLCluster* cluster, const TVector3& vertex,
                                                 ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 4x4 covariance matrix (px, py, pz, E)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 4x4 covariance matrix (px, py, pz, E)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster,
                                                        const TMatrixDSym& covmatvertex, ECLCluster::EHypothesisBit hypo,
                                                        const TMatrixD& jacobiMatrix);

    /**
     * Returns 7x7 covariance matrix (px, py, pz, E, x, y, z)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 7x7 covariance matrix (px, py, pz, E, x, y, z)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster,
                                                        const TMatrixDSym& covmatvertex, ECLCluster::EHypothesisBit hypo,
                                                        const TMatrixD& jacobiMatrix);

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

    /**
     * Returns Jacobian matrix
     * @return const TMatrixD
     */
    const TMatrixD jacobiMatrix;
  private:

    DBObjPtr<BeamSpot> m_beamSpotDB; /**< Beam spot database object */
  };

} // Belle2 namespace
