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

TVector3 ECLCluster::getClusterPosition() const
{
  const double cluster_x =  getR() * sin(getTheta()) * cos(getPhi());
  const double cluster_y =  getR() * sin(getTheta()) * sin(getPhi());
  const double cluster_z =  getR() * cos(getTheta());
  return TVector3(cluster_x, cluster_y, cluster_z);
}

TMatrixDSym ECLCluster::getCovarianceMatrix3x3() const
{
  TMatrixDSym covmatecl(3);
  covmatecl(0, 0) = m_sqrtcovmat_00 * m_sqrtcovmat_00;
  covmatecl(1, 0) = m_covmat_10;
  covmatecl(1, 1) = m_sqrtcovmat_11 * m_sqrtcovmat_11;
  covmatecl(2, 0) = m_covmat_20;
  covmatecl(2, 1) = m_covmat_21;
  covmatecl(2, 2) = m_sqrtcovmat_22 * m_sqrtcovmat_22;

  //make symmetric
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < i ; j++)
      covmatecl(j, i) = covmatecl(i, j);
  return covmatecl;
}

void ECLCluster::getCovarianceMatrixAsArray(double covArray[6]) const
{
  covArray[0] = m_sqrtcovmat_00 * m_sqrtcovmat_00;
  covArray[1] = m_covmat_10;
  covArray[2] = m_sqrtcovmat_11 * m_sqrtcovmat_11;
  covArray[3] = m_covmat_20;
  covArray[4] = m_covmat_21;
  covArray[5] = m_sqrtcovmat_22 * m_sqrtcovmat_22;
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

int ECLCluster::getUniqueId() const
{
  const int crid     = getConnectedRegionId();
  const int showerid = getClusterId();
  const int hypoid   = getHypothesisId();

  return 100000 * crid + 1000 * hypoid + showerid;
}
