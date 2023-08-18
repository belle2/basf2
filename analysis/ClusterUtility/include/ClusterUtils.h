/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// DATABASE objects
#include <mdst/dbobjects/BeamSpot.h>
#include <analysis/dbobjects/ECLPhotonEnergyResolution.h>
#include <framework/database/DBObjPtr.h>

// MDST
#include <mdst/dataobjects/ECLCluster.h>

// ROOT
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
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
    const ROOT::Math::PxPyPzEVector GetCluster4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns four momentum vector
     * @return const four momentum vector
     */
    const ROOT::Math::PxPyPzEVector Get4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns four momentum vector
     * @return const four momentum vector
     */
    const ROOT::Math::PxPyPzEVector Get4MomentumFromCluster(const ECLCluster* cluster, const ROOT::Math::XYZVector& vertex,
                                                            ECLCluster::EHypothesisBit hypo);
    /**
     * Returns 4x6 Jacobi matrix (px, py, pz, E)
     * @return const TMatrixD
     */
    const TMatrixD GetJacobiMatrix4x6FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 4x6 Jacobi matrix (px, py, pz, E)
     * @return const TMatrixD
     */
    const TMatrixD GetJacobiMatrix4x6FromCluster(const ECLCluster* cluster, const ROOT::Math::XYZVector& vertex,
                                                 ECLCluster::EHypothesisBit hypo);

    /**
     * Returns 3x3 covariance matrix (E, theta, phi)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix3x3FromCluster(const ECLCluster* cluster);

    /**
     * Returns 4x4 covariance matrix (px, py, pz, E)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster, const TMatrixD& jacobiMatrix);

    /**
     * Returns 4x4 covariance matrix (px, py, pz, E)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster,
                                                        const TMatrixDSym& covmatvertex,
                                                        const TMatrixD& jacobiMatrix);

    /**
     * Returns 7x7 covariance matrix (px, py, pz, E, x, y, z)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster, const TMatrixD& jacobiMatrix);

    /**
     * Returns 7x7 covariance matrix (px, py, pz, E, x, y, z)
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster,
                                                        const TMatrixDSym& covmatvertex,
                                                        const TMatrixD& jacobiMatrix);

    /**
     * Returns default IP position from beam parameters
     * @return const XYZVector
     */
    const ROOT::Math::XYZVector GetIPPosition();

    /**
     * Returns default IP position covariance matrix from beam parameters
     * @return const TMatrixDSym
     */
    const TMatrixDSym GetIPPositionCovarianceMatrix();

  private:
    /**
     * Beam spot database object
     */
    DBObjPtr<BeamSpot> m_beamSpotDB;

    /**
     * Photon energy resolution database object
     */
    DBObjPtr<ECLPhotonEnergyResolution> m_photonEnergyResolutionDB;
  };

} // Belle2 namespace
