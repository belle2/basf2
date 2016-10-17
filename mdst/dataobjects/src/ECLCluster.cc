/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TMatrixD.h>

/* Belle2 headers. */
#include <mdst/dataobjects/ECLCluster.h>

using namespace Belle2;

TVector3 ECLCluster::getMomentum() const
{
  return TVector3(getPx(), getPy(), getPz());
}

TLorentzVector ECLCluster::get4Vector() const
{
  return TLorentzVector(getPx(), getPy(), getPz(), getEnergy());
}

TVector3 ECLCluster::getClusterPosition() const
{
  const double cluster_x =  getR() * sin(getTheta()) * cos(getPhi());
  const double cluster_y =  getR() * sin(getTheta()) * sin(getPhi());
  const double cluster_z =  getR() * cos(getTheta());
  return TVector3(cluster_x, cluster_y, cluster_z);
}

TVector3 ECLCluster::getPosition() const
{
  return TVector3(0, 0, 0);
}

TMatrixDSym ECLCluster::getCovarianceMatrix4x4() const
{
  TMatrixDSym covmatecl = getCovarianceMatrix3x3();

  TMatrixD jacobian(4, 3);
  const double cosPhi = cos(getPhi());
  const double sinPhi = sin(getPhi());
  const double cosTheta = cos(getTheta());
  const double sinTheta = sin(getTheta());
  const double E = getEnergy();

  jacobian(0, 0) =            cosPhi * sinTheta;
  jacobian(0, 1) = -1.0 * E * sinPhi * sinTheta;
  jacobian(0, 2) =        E * cosPhi * cosTheta;
  jacobian(1, 0) =            sinPhi * sinTheta;
  jacobian(1, 1) =        E * cosPhi * sinTheta;
  jacobian(1, 2) =        E * sinPhi * cosTheta;
  jacobian(2, 0) =                     cosTheta;
  jacobian(2, 1) =           0.0;
  jacobian(2, 2) = -1.0 * E          * sinTheta;
  jacobian(3, 0) =           1.0;
  jacobian(3, 1) =           0.0;
  jacobian(3, 2) =           0.0;
  TMatrixDSym covmatCart(4);
  covmatCart = covmatecl.Similarity(jacobian);
  return covmatCart;
}

TMatrixDSym ECLCluster::getCovarianceMatrix7x7() const
{
  const TMatrixDSym covmatCart = getCovarianceMatrix4x4();

  TMatrixDSym covmatMatrix(7);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      covmatMatrix(i, j) = covmatMatrix(j, i) = covmatCart(i, j);
    }
  }
  for (int i = 4; i < 7; ++i) {
    covmatMatrix(i, i) = 1.0; // 1.0*1.0 cm^2 (default treatment as Belle ?)
  }
  return covmatMatrix;
}

TMatrixDSym ECLCluster::getCovarianceMatrix3x3() const
{
  TMatrixDSym covmatecl(3);
  covmatecl(0, 0) = m_covmat_00;
  covmatecl(1, 0) = m_covmat_10;
  covmatecl(1, 1) = m_covmat_11;
  covmatecl(2, 0) = m_covmat_20;
  covmatecl(2, 1) = m_covmat_21;
  covmatecl(2, 2) = m_covmat_22;

  //make symmetric
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < i ; j++)
      covmatecl(j, i) = covmatecl(i, j);
  return covmatecl;
}

void ECLCluster::getCovarianceMatrixAsArray(double covArray[6]) const
{
  covArray[0] = m_covmat_00;
  covArray[1] = m_covmat_10;
  covArray[2] = m_covmat_11;
  covArray[3] = m_covmat_20;
  covArray[4] = m_covmat_21;
  covArray[5] = m_covmat_22;
}


int ECLCluster::getDetectorRegion() const
{
  const double theta = getTheta();

  if (theta < 0.2164208) return 0;   // < 12.4deg
  if (theta < 0.5480334) return 1;   // < 31.4deg
  if (theta < 0.561996) return 11;   // < 32.2deg
  if (theta < 2.2462387) return 2;   // < 128.7
  if (theta < 2.2811453) return 13;   // < 130.7
  if (theta < 2.7070057) return 3;   // < 155.1
  else return 0;
}
