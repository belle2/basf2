/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/ClusterUtility/ClusterUtils.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace ROOT::Math;

// -----------------------------------------------------------------------------
ClusterUtils::ClusterUtils() = default;

// -----------------------------------------------------------------------------
const PxPyPzEVector ClusterUtils::GetCluster4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo)
{
  // Use the geometry origin (0,0,0) and *not* the IP position
  return Get4MomentumFromCluster(cluster, XYZVector(0.0, 0.0, 0.0), hypo);
}

const PxPyPzEVector ClusterUtils::Get4MomentumFromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo)
{

  // Use the default vertex from the beam parameters if none is given.
  return Get4MomentumFromCluster(cluster, GetIPPosition(), hypo);
}

const PxPyPzEVector ClusterUtils::Get4MomentumFromCluster(const ECLCluster* cluster, const ROOT::Math::XYZVector& vertex,
                                                          ECLCluster::EHypothesisBit hypo)
{

  // Get particle direction from vertex and reconstructed cluster position.
  XYZVector direction(cluster->getClusterPosition().X() - vertex.X(),
                      cluster->getClusterPosition().Y() - vertex.Y(),
                      cluster->getClusterPosition().Z() - vertex.Z());

  // Always ignore mass here (even for neutral hadrons) therefore the magnitude
  // of the momentum is equal to the cluster energy under this hypo.
  const double E = cluster->getEnergy(hypo); // must not be changed or clusterE getters will be wrong
  const double px = E * sin(direction.Theta()) * cos(direction.Phi());
  const double py = E * sin(direction.Theta()) * sin(direction.Phi());
  const double pz = E * cos(direction.Theta());

  const PxPyPzEVector l(px, py, pz, E);
  return l;
}

// -----------------------------------------------------------------------------

const TMatrixD ClusterUtils::GetJacobiMatrix4x6FromCluster(const ECLCluster* cluster, ECLCluster::EHypothesisBit hypo)
{

  return GetJacobiMatrix4x6FromCluster(cluster, GetIPPosition(), hypo);
}

const TMatrixD ClusterUtils::GetJacobiMatrix4x6FromCluster(const ECLCluster* cluster, const ROOT::Math::XYZVector& vertex,
                                                           ECLCluster::EHypothesisBit hypo)
{

  // Calculate the Jacobi matrix J =  dpx/dE dpx/dphi dpx/dtheta dpx/dx dpy/dy dpz/dz
  //                                  ...
  //                                  dpx/dE dE/dphi dE/dtheta dpx/dx dE/dy dE/dz
  TMatrixD jacobian(4, 6);

  const double R = cluster->getR();
  const double energy = cluster->getEnergy(hypo);
  const double theta = cluster->getTheta();
  const double phi = cluster->getPhi();

  const double st = sin(theta);
  const double ct = cos(theta);
  const double sp = sin(phi);
  const double cp = cos(phi);

  const double clx = R * st * cp; // cluster position x
  const double cly = R * st * sp; // cluster position y
  const double clz = R * ct;      // cluster position z

  const double vx = vertex.X(); // vertex position x
  const double vy = vertex.Y(); // vertex position y
  const double vz = vertex.Z(); // vertex position z

  const double dx = clx - vx; // direction vector x
  const double dy = cly - vy; // direction vector y
  const double dz = clz - vz; // direction vector z
  const double dx2 = dx * dx; // direction vector x squared
  const double dy2 = dy * dy; // direction vector y squared
  const double dz2 = dz * dz; // direction vector z squared

  const double r2 = (dx * dx + dy * dy + dz * dz);
  const double r12 = sqrt(r2);
  const double r32 = pow(r2, 1.5);

  // px = E * dx/r
  jacobian(0, 0) = dx / r12;                                                                              // dpx/dE
  jacobian(0, 1) = -energy * R * ((dx * dy * cp) + ((dy2 + dz2) * sp)) * st / (r32);                      // dpx/dphi
  jacobian(0, 2) = energy * R * (((dy2 + dz2) * cp * ct) - (dx * dy * ct * sp) + (dx * dz * st)) / (r32); // dpx/dtheta
  jacobian(0, 3) = -energy * (dy2 + dz2) / (r32);                                                         // dpx/dvx
  jacobian(0, 4) = energy * dx * dy / (r32);                                                              // dpx/dvy
  jacobian(0, 5) = energy * dx * dz / (r32);                                                              // dpx/dvz

  // py = E * dy/r
  jacobian(1, 0) = dy / r12;                                                                               // dpy/dE
  jacobian(1, 1) = energy * R * (((dx2 + dz2) * cp) + (dx * dy * sp)) * st / (r32);                        // dpy/dphi
  jacobian(1, 2) = energy * R * ((-dx * dy * cp * ct) + ((dx2 + dz2) * ct * sp) + (dy * dz * st)) / (r32); // dpz/dtheta
  jacobian(1, 3) = energy * dx * dy / (r32);                                                               // dpy/dvx
  jacobian(1, 4) = -energy * (dx2 + dz2) / (r32);                                                          // dpy/dvy
  jacobian(1, 5) = energy * dy * dz / (r32);                                                               // dpy/dvz

  // pz = E * dz/r
  jacobian(2, 0) = dz / r12;                                                                             // dpz/dE
  jacobian(2, 1) = energy * R * dz * (-dy * cp + dx * sp) * st / (r32);                                  // dpz/dphi
  jacobian(2, 2) = -energy * R * ((dx * dz * cp * ct) + (dy * dz * ct * sp) + (dx2 + dy2) * st) / (r32); // dpz/dtheta
  jacobian(2, 3) = energy * dx * dz / (r32);                                                             // dpz/dvx
  jacobian(2, 4) = energy * dy * dz / (r32);                                                             // dpz/dvy
  jacobian(2, 5) = -energy * (dx2 + dy2) / (r32);                                                        // dpz/ydvz

  // E
  jacobian(3, 0) = 1.0; // dE/dE
  jacobian(3, 1) = 0.0; // dE/dphi
  jacobian(3, 2) = 0.0; // dE/dtheta
  jacobian(3, 3) = 0.0; // dE/dvx
  jacobian(3, 4) = 0.0; // dE/dvy
  jacobian(3, 5) = 0.0; // dE/dvz

  // jacobian.Print();
  return jacobian;
}

const TMatrixDSym ClusterUtils::GetCovarianceMatrix3x3FromCluster(const ECLCluster* cluster)
{
  // Get the covariance matrix (energy, phi, theta) from the ECL cluster.
  TMatrixDSym covmatecl = cluster->getCovarianceMatrix3x3();

  /**
   * Replace fixed energy resolution value by derived value from the DB if present
   */
  if (m_photonEnergyResolutionDB.isValid() and cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
    double pEnergy = cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
    double pTheta = cluster->getTheta();
    double pPhi = cluster->getPhi();

    double energyCovarianceElement = m_photonEnergyResolutionDB->getRelativeEnergyResolution(pEnergy, pTheta, pPhi);
    if (energyCovarianceElement != -1) {
      covmatecl(0, 0) = energyCovarianceElement * pEnergy * energyCovarianceElement * pEnergy;
    }
  }

  return covmatecl;
}

const TMatrixDSym ClusterUtils::GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster, const TMatrixD& jacobiMatrix)
{

  return GetCovarianceMatrix4x4FromCluster(cluster, GetIPPositionCovarianceMatrix(), jacobiMatrix);
}

const TMatrixDSym ClusterUtils::GetCovarianceMatrix4x4FromCluster(const ECLCluster* cluster, const TMatrixDSym& covmatvertex,
    const TMatrixD& jacobiMatrix)
{

  // Get the covariance matrix (energy, phi, theta)
  TMatrixDSym covmatecl = GetCovarianceMatrix3x3FromCluster(cluster);

  // Combine into the total covariance matrix from the ECL cluster (0..2) and the IP(3..5) as two 3x3 block matrices.
  TMatrixDSym covmatcombined(6);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j <= i; j++) {
      covmatcombined(i, j) = covmatcombined(j, i) = covmatecl(i, j);
      covmatcombined(i + 3, j + 3) = covmatcombined(j + 3, i + 3) = covmatvertex(i, j);
    }
  }

  TMatrixDSym covmatCart(4);
  covmatCart = covmatcombined.Similarity(jacobiMatrix);
  return covmatCart;
}

// -----------------------------------------------------------------------------

const TMatrixDSym ClusterUtils::GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster, const TMatrixD& jacobiMatrix)
{

  return GetCovarianceMatrix7x7FromCluster(cluster, GetIPPositionCovarianceMatrix(), jacobiMatrix);
}

const TMatrixDSym ClusterUtils::GetCovarianceMatrix7x7FromCluster(const ECLCluster* cluster,
    const TMatrixDSym& covmatvertex, const TMatrixD& jacobiMatrix)
{
  TMatrixDSym covmat4x4 = GetCovarianceMatrix4x4FromCluster(cluster, covmatvertex, jacobiMatrix);

  TMatrixDSym covmatCart(7);

  // px, py, pz, E
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i; j++) {
      covmatCart(i, j) = covmatCart(j, i) = covmat4x4(i, j);
    }
  }

  // x, y, z
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j <= i; j++) {
      covmatCart(i + 4, j + 4) = covmatCart(j + 4, i + 4) = covmatvertex(i, j);
    }
  }

  return covmatCart;
}

// -----------------------------------------------------------------------------
const XYZVector ClusterUtils::GetIPPosition()
{
  if (!m_beamSpotDB) {
    B2WARNING("Beamspot not available, using (0, 0, 0) as IP position instead.");
    return XYZVector(0.0, 0.0, 0.0);
  } else
    return XYZVector(m_beamSpotDB->getIPPosition().X(), m_beamSpotDB->getIPPosition().Y(), m_beamSpotDB->getIPPosition().Z());
}

// -----------------------------------------------------------------------------
const TMatrixDSym ClusterUtils::GetIPPositionCovarianceMatrix()
{
  if (!m_beamSpotDB) {
    B2WARNING("Beam parameters not available, using ((1, 0, 0), (0, 1, 0), (0, 0, 1)) as IP covariance matrix instead.");

    TMatrixDSym covmat(3);
    for (int i = 0; i < 3; ++i) {
      covmat(i, i) = 1.0; // 1.0*1.0 cm^2
    }
    return covmat;
  } else
    return m_beamSpotDB->getCovVertex();
}
