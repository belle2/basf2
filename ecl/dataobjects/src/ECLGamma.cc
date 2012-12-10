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
  TVector3 momentum(0., 0., 0.);
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  double m_energy = aECLShower->GetEnergy();
  double m_theta = aECLShower->GetTheta();
  double m_phi = aECLShower->GetPhi();
  double m_px = m_energy * sin(m_theta) * cos(m_phi);
  double m_py = m_energy * sin(m_theta) * sin(m_phi);
  double m_pz = m_energy * cos(m_theta);

  momentum.SetX(m_px);
  momentum.SetY(m_py);
  momentum.SetZ(m_pz);
  return momentum;
}

float ECLGamma::getPx() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  double m_energy = aECLShower->GetEnergy();
  double m_theta = aECLShower->GetTheta();
  double m_phi = aECLShower->GetPhi();
  double m_px = m_energy * sin(m_theta) * cos(m_phi);
  return (float)m_px;
}


float ECLGamma::getPy() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  double m_energy = aECLShower->GetEnergy();
  double m_theta = aECLShower->GetTheta();
  double m_phi = aECLShower->GetPhi();
  double m_py = m_energy * sin(m_theta) * sin(m_phi);
  return (float)m_py;
}

float ECLGamma::getPz() const
{
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  double m_energy = aECLShower->GetEnergy();
  double m_theta = aECLShower->GetTheta();
  double m_pz = m_energy * cos(m_theta);

  return (float)m_pz;
}


float ECLGamma::getEnergy() const
{
  TVector3 momentum(0., 0., 0.);
  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  double m_energy = aECLShower->GetEnergy();
  return (float)m_energy;
}



void ECLGamma::getErrorMatrix(TMatrixFSym& m_errorMatrix) const
{

  StoreArray<ECLShower> eclRecShowerArray;
  ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
  double EnergyError = aECLShower->GetEnergyError();
  double ThetaError = aECLShower->GetThetaError();
  double PhiError = aECLShower->GetPhiError();
  double m_energy = aECLShower->GetEnergy();
  double m_theta = aECLShower->GetTheta();
  double m_phi = aECLShower->GetPhi();

  CLHEP::HepSymMatrix  errEcl(3, 0);   // 3x3 initialize to zero
  errEcl[ 0 ][ 0 ] = EnergyError * EnergyError; // Energy
  errEcl[ 1 ][ 0 ] = 0;
  errEcl[ 1 ][ 1 ] = PhiError * PhiError; // Phi
  errEcl[ 2 ][ 0 ] = 0;
  errEcl[ 2 ][ 0 ] = 0;
  errEcl[ 2 ][ 1 ] = 0;
  errEcl[ 2 ][ 2 ] = ThetaError * ThetaError; // Theta

  CLHEP::HepMatrix  jacobian(4, 3, 0);
  double  cosPhi = cos(m_phi);
  double  sinPhi = sin(m_phi);
  double  cosTheta = cos(m_theta);
  double  sinTheta = sin(m_theta);
  double   E = m_energy;

  jacobian[ 0 ][ 0 ] =       cosPhi * sinTheta;
  jacobian[ 0 ][ 1 ] =  -E * sinPhi * sinTheta;
  jacobian[ 0 ][ 2 ] =   E * cosPhi * cosTheta;
  jacobian[ 1 ][ 0 ] =       sinPhi * sinTheta;
  jacobian[ 1 ][ 1 ] =   E * cosPhi * sinTheta;
  jacobian[ 1 ][ 2 ] =   E * sinPhi * cosTheta;
  jacobian[ 2 ][ 0 ] =            cosTheta;
  jacobian[ 2 ][ 1 ] =           0.0;
  jacobian[ 2 ][ 2 ] =  -E      * sinTheta;
  jacobian[ 3 ][ 0 ] =           1.0;
  jacobian[ 3 ][ 1 ] =           0.0;
  jacobian[ 3 ][ 2 ] =           0.0;
  CLHEP::HepSymMatrix errCart(4, 0);
  errCart = errEcl.similarity(jacobian);


  //std::cout<<"Gamma ErrorMatrixA ";
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      m_errorMatrix[i][j] = errCart[i][j];
      //std::cout<<m_errorMatrix[i][j]<<" ";
    }
  }
  //std::cout<<std::endl;
}

/*
TMatrixT<float> ECLGamma::getErrorMatrix()  {
      m_errorMatrix.ResizeTo(4, 4);
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i; j++) {
          m_errorMatrix[i][j] = 0;
        }
      }

      StoreArray<ECLShower> eclRecShowerArray;
      ECLShower* aECLShower = eclRecShowerArray[m_ShowerId];
      double EnergyError = aECLShower->GetEnergyError();
      double ThetaError = aECLShower->GetThetaError();
      double PhiError = aECLShower->GetPhiError();
      double m_energy = aECLShower->GetEnergy();
      double m_theta = aECLShower->GetTheta();
      double m_phi = aECLShower->GetPhi();


      CLHEP::HepSymMatrix  errEcl(3, 0);   // 3x3 initialize to zero
      errEcl[ 0 ][ 0 ] = EnergyError * EnergyError; // Energy
      errEcl[ 1 ][ 0 ] = 0;
      errEcl[ 1 ][ 1 ] = PhiError * PhiError; // Phi
      errEcl[ 2 ][ 0 ] = 0;
      errEcl[ 2 ][ 1 ] = 0;
      errEcl[ 2 ][ 2 ] = ThetaError * ThetaError; // Theta

      CLHEP::HepMatrix  jacobian(4, 3, 0);
      double  cosPhi = cos(m_phi);
      double  sinPhi = sin(m_phi);
      double  cosTheta = cos(m_theta);
      double  sinTheta = sin(m_theta);
      double   E = m_energy;

      jacobian[ 0 ][ 0 ] =       cosPhi * sinTheta;
      jacobian[ 0 ][ 1 ] =  -E * sinPhi * sinTheta;
      jacobian[ 0 ][ 2 ] =   E * cosPhi * cosTheta;
      jacobian[ 1 ][ 0 ] =       sinPhi * sinTheta;
      jacobian[ 1 ][ 1 ] =   E * cosPhi * sinTheta;
      jacobian[ 1 ][ 2 ] =   E * sinPhi * cosTheta;
      jacobian[ 2 ][ 0 ] =            cosTheta;
      jacobian[ 2 ][ 1 ] =           0.0;
      jacobian[ 2 ][ 2 ] =  -E      * sinTheta;
      jacobian[ 3 ][ 0 ] =           1.0;
      jacobian[ 3 ][ 1 ] =           0.0;
      jacobian[ 3 ][ 2 ] =           0.0;
      CLHEP::HepSymMatrix errCart(4, 0);
      errCart = errEcl.similarity(jacobian);


      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          m_errorMatrix[i][j] = errCart[i][j];
        }
      }
      return m_errorMatrix;
    }
*/



ClassImp(ECLGamma)
