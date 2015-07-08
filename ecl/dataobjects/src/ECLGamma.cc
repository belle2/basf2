/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "ecl/dataobjects/ECLGamma.h"

using namespace std;
using namespace Belle2;

TVector3 ECLGamma::getMomentum() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  const double energy = aECLShower->getEnergy();
  const double theta = aECLShower->getTheta();
  const double phi = aECLShower->getPhi();
  const double px = energy * sin(theta) * cos(phi);
  const double py = energy * sin(theta) * sin(phi);
  const double pz = energy * cos(theta);

  return TVector3(px, py, pz);
}

float ECLGamma::getPx() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  const double energy = aECLShower->getEnergy();
  const double theta = aECLShower->getTheta();
  const double phi = aECLShower->getPhi();
  return energy * sin(theta) * cos(phi);
}


float ECLGamma::getPy() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  const double energy = aECLShower->getEnergy();
  const double theta = aECLShower->getTheta();
  const double phi = aECLShower->getPhi();
  return energy * sin(theta) * sin(phi);
}

float ECLGamma::getPz() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  const double energy = aECLShower->getEnergy();
  const double theta = aECLShower->getTheta();
  return energy * cos(theta);
}


float ECLGamma::getEnergy() const
{
  TVector3 momentum(0., 0., 0.);
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  return aECLShower->getEnergy();
}



void ECLGamma::getErrorMatrix(TMatrixFSym& m_errorMatrix) const
{

  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  const double EnergyError = aECLShower->getEnergyError();
  const double ThetaError = aECLShower->getThetaError();
  const double PhiError = aECLShower->getPhiError();
  const double energy = aECLShower->getEnergy();
  const double theta = aECLShower->getTheta();
  const double phi = aECLShower->getPhi();

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[0][0] = EnergyError * EnergyError; // Energy
  errEcl[1][0] = 0;
  errEcl[1][1] = PhiError * PhiError; // Phi
  errEcl[2][0] = 0;
  errEcl[2][0] = 0;
  errEcl[2][1] = 0;
  errEcl[2][2] = ThetaError * ThetaError; // Theta

  TMatrixF  jacobian(4, 3);
  const double cosPhi = cos(phi);
  const double sinPhi = sin(phi);
  const double cosTheta = cos(theta);
  const double sinTheta = sin(theta);
  const double E = energy;

  jacobian[0][0] =       cosPhi * sinTheta;
  jacobian[0][1] =  -E * sinPhi * sinTheta;
  jacobian[0][2] =   E * cosPhi * cosTheta;
  jacobian[1][0] =       sinPhi * sinTheta;
  jacobian[1][1] =   E * cosPhi * sinTheta;
  jacobian[1][2] =   E * sinPhi * cosTheta;
  jacobian[2][0] =            cosTheta;
  jacobian[2][1] =           0.0;
  jacobian[2][2] =  -E      * sinTheta;
  jacobian[3][0] =           1.0;
  jacobian[3][1] =           0.0;
  jacobian[3][2] =           0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      m_errorMatrix[i][j] = errCart[i][j];
    }
  }
}

ClassImp(ECLGamma)
