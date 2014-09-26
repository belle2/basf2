/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Giulia Casarosa                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleFlightInfoTool.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleFlightInfoTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  //flight distance
  m_tree->Branch((strNames[1] + "_FD").c_str(), &m_fD, (strNames[1] + "FD/F").c_str());
  m_tree->Branch((strNames[1] + "_FDE").c_str(), &m_fDE, (strNames[1] + "_FDE/F").c_str());

  //flight time
  m_tree->Branch((strNames[1] + "_FT").c_str(), &m_fT, (strNames[1] + "FT/F").c_str());
  m_tree->Branch((strNames[1] + "_FTE").c_str(), &m_fTE, (strNames[1] + "_FTE/F").c_str());

}

void NtupleFlightInfoTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleFlightInfoTool::eval - ERROR, no Particle found!\n");
    return;
  }


  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 2) {
    printf("NtupleFlightInfoTool::eval - ERROR, you must to select exactly 2 particles in the decay (mother and daughter)!\n");
    return;
  }

  //get the MOTHER
  const Particle*  mother = selparticles[0];

  //get the DAUGHTER
  const Particle*  daughter = selparticles[1];

  evalFlightDistance(mother, daughter);

  evalFlightTime(mother, daughter);

  return;

}



void NtupleFlightInfoTool::evalFlightDistance(const Particle* mother, const Particle* daughter)
{

  TMatrixFSym mumCov = mother->getVertexErrorMatrix();   //order: x,y,z

  //mother vertex
  double mumvtxX = mother->getX();
  double mumvtxY = mother->getY();
  double mumvtxZ = mother->getZ();

  //daughter vertex
  double vtxX =  daughter->getX();
  double vtxY =  daughter->getY();
  double vtxZ =  daughter->getZ();

  // daughter MOMENTUM
  double p = daughter->getP();
  double pX = daughter->getPx();
  double pY = daughter->getPy();
  double pZ = daughter->getPz();

  //versor of the daughter momentum
  double nX = pX / p;
  double nY = pY / p;
  double nZ = pZ / p;

  //Distance between mother and daughter vertices
  double lX = vtxX - mumvtxX;
  double lY = vtxY - mumvtxY;
  double lZ = vtxZ - mumvtxZ;

  //flight distance
  m_fD      = lX * nX + lY * nY + lZ * nZ;

  //covariance matrix of momentum and vertex for the Dz
  //ORDER = px,py,pz,E,x,y,z
  TMatrixFSym dauCov = daughter->getMomentumVertexErrorMatrix();

  //compute total covariance matrix
  //ORDER = px dau, py dau, pz dau, E dau, x dau, y dau, z dau, x mum, y mum, z mum

  TMatrixFSym Cov(10);
  for (int i = 0; i < 10; i++)
    for (int j = 0; j < 10; j++)
      if (i < 7 && j < 7)
        Cov[i][j] = dauCov[i][j];
      else if (i > 6 && j > 6)
        Cov[i][j] = mumCov[i - 7][j - 7];
      else
        Cov[i][j] = 0;

  TMatrixF deriv(10, 1);
  deriv[0][0] = (lX - nX * m_fD) / p;   //px Daughter
  deriv[1][0] = (lY - nY * m_fD) / p;   //py Daughter
  deriv[2][0] = (lZ - nZ * m_fD) / p;   //pz Daughter
  deriv[3][0] = 0; //E Daughter
  deriv[4][0] = nX;   //vtxX Daughter
  deriv[5][0] = nY;   //vtxY Daughter
  deriv[6][0] = nZ;   //vtxZ Daughter
  deriv[7][0] = - nX;   //vtxX Mother
  deriv[8][0] = - nY;   //vtxY Mother
  deriv[9][0] = - nZ;   //vtxZ Mother


  TMatrixF tmp(10, 1);
  tmp.Mult(Cov, deriv);

  TMatrixF result(1, 1);
  result.Mult(deriv.T(), tmp);

  m_fDE = sqrt(result[0][0]);

}

void NtupleFlightInfoTool::evalFlightTime(const Particle* mother, const Particle* daughter)
{

  TMatrixFSym mumCov = mother->getVertexErrorMatrix();   //order: x,y,z

  //mother vertex
  double mumvtxX = mother->getX();
  double mumvtxY = mother->getY();
  double mumvtxZ = mother->getZ();

  //daughter vertex
  double vtxX =  daughter->getX();
  double vtxY =  daughter->getY();
  double vtxZ =  daughter->getZ();

  // daughter MOMENTUM
  double p = daughter->getP();
  double pX = daughter->getPx();
  double pY = daughter->getPy();
  double pZ = daughter->getPz();

  //Distance between mother and daughter vertices
  double lX = vtxX - mumvtxX;
  double lY = vtxY - mumvtxY;
  double lZ = vtxZ - mumvtxZ;

  //flight time
  m_fT = daughter->getPDGMass() / Const::speedOfLight * m_fD / p;

  //covariance matrix of momentum and vertex for the Dz
  //ORDER = px,py,pz,E,x,y,z
  TMatrixFSym dauCov = daughter->getMomentumVertexErrorMatrix();

  //compute total covariance matrix
  //ORDER = px dau, py dau, pz dau, E dau, x dau, y dau, z dau, x mum, y mum, z mum

  TMatrixFSym Cov(10);
  for (int i = 0; i < 10; i++)
    for (int j = 0; j < 10; j++)
      if (i < 7 && j < 7)
        Cov[i][j] = dauCov[i][j];
      else if (i > 6 && j > 6)
        Cov[i][j] = mumCov[i - 7][j - 7];
      else
        Cov[i][j] = 0;


  TMatrixF deriv(10, 1);
  deriv[0][0] = (daughter->getPDGMass() / Const::speedOfLight * lX - 2 * pX * m_fT) / p / p; //px Daughter
  deriv[1][0] = (daughter->getPDGMass() / Const::speedOfLight * lY - 2 * pY * m_fT) / p / p; //py Daughter
  deriv[2][0] = (daughter->getPDGMass() / Const::speedOfLight * lZ - 2 * pZ * m_fT) / p / p; //pz Daughter
  deriv[3][0] = 0; //E Daughter
  deriv[4][0] = daughter->getPDGMass() / Const::speedOfLight * pX / p / p; //vtxX Daughter
  deriv[5][0] = daughter->getPDGMass() / Const::speedOfLight * pY / p / p; //vtxY Daughter
  deriv[6][0] = daughter->getPDGMass() / Const::speedOfLight * pZ / p / p; //vtxZ Daughter
  deriv[7][0] = - daughter->getPDGMass() / Const::speedOfLight * pX / p / p; //vtxX Mother
  deriv[8][0] = - daughter->getPDGMass() / Const::speedOfLight * pY / p / p; //vtxY Mother
  deriv[9][0] = - daughter->getPDGMass() / Const::speedOfLight * pZ / p / p; //vtxZ Mother


  TMatrixF tmp(10, 1);
  tmp.Mult(Cov, deriv);

  TMatrixF result(1, 1);
  result.Mult(deriv.T(), tmp);

  m_fTE = sqrt(result[0][0]);

  return;

}
