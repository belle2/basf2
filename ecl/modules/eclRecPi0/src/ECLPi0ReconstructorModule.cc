/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecPi0/ECLPi0ReconstructorModule.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <ecl/dataobjects/ECLGamma.h>

#include <ecl/dataobjects/ECLShower.h>
#include <framework/datastore/RelationArray.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//#include<ctime>
//#include <iomanip>
#include <float.h>


// ROOT
#include  "CLHEP/Vector/ThreeVector.h"
#include  "CLHEP/Vector/LorentzVector.h"
#include  "CLHEP/Matrix/Matrix.h"
#include <TMatrixFSym.h>


#include <analysis/KFit/MakeMotherKFit.h>

#define PI 3.14159265358979323846

using namespace std;
using namespace Belle2;
using namespace Belle2::analysis;
using namespace Belle2::ECL;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLPi0Reconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLPi0ReconstructorModule::ECLPi0ReconstructorModule() : Module()
{
  //Set module properties
  setDescription("Creates ECL_pi0 from ECL_gamma.");
  setPropertyFlags(c_ParallelProcessingCertified);
}


ECLPi0ReconstructorModule::~ECLPi0ReconstructorModule()
{

}

void ECLPi0ReconstructorModule::initialize()
{

  pi0_mass_min = 0.08; // Modify: 20040112 KM
  pi0_mass_max = 0.18; // Modify: 20040112 KM
  chi2_max = 15.;
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;
  StoreArray<ECLPi0>::registerPersistent();

  RelationArray::registerPersistent<ECLPi0, ECLGamma>("", "");
}

void ECLPi0ReconstructorModule::beginRun()
{
}


void ECLPi0ReconstructorModule::event()
{

  StoreArray<ECLGamma> Gamma;
  StoreArray<ECLShower> eclRecShowerArray;
  RelationArray eclGammaToShower(Gamma, eclRecShowerArray);
  StoreArray<ECLPi0> Pi0Array;
  RelationArray eclPi0ToGamma(Pi0Array, Gamma);

  for (int iGamma = 0; iGamma < eclGammaToShower.getEntries(); iGamma++) {
    if (eclGammaToShower[iGamma].getToIndices().size() != 1) {
      B2ERROR("Relation Gamma To Shower is not only one in event " << m_nEvent << " nShower " << eclGammaToShower[iGamma].getToIndices().size());
    }
  }

  for (int iGamma1 = 0; iGamma1 < Gamma.getEntries() - 1; iGamma1++) {
    ECLGamma* aECLGamma1 = Gamma[iGamma1];
    double EGamma1 =  aECLGamma1->getEnergy();
    m_showerId1 = aECLGamma1->getShowerId();
    m_px1 = aECLGamma1->getPx();
    m_py1 = aECLGamma1->getPy();
    m_pz1 = aECLGamma1->getPz();



    CLHEP::HepSymMatrix   errorGamma1(7, 0);
    readErrorMatrix(iGamma1, errorGamma1);
    CLHEP::Hep3Vector p3Gamma1(m_px1, m_py1, m_pz1);

    for (int iGamma2 = iGamma1 + 1; iGamma2 < Gamma.getEntries(); iGamma2++) {
      ECLGamma* aECLGamma2 = Gamma[iGamma2];
      double EGamma2 =  aECLGamma2->getEnergy();
      m_showerId2 = aECLGamma2->getShowerId();
      m_px2 = aECLGamma2->getPx();
      m_py2 = aECLGamma2->getPy();
      m_pz2 = aECLGamma2->getPz();
      CLHEP::HepSymMatrix   errorGamma2(7, 0);
      readErrorMatrix(iGamma2, errorGamma2);
      CLHEP::Hep3Vector p3Gamma2(m_px2, m_py2, m_pz2);

      CLHEP::HepLorentzVector lv_gamma1(p3Gamma1, EGamma1);
      CLHEP::HepLorentzVector lv_gamma2(p3Gamma2, EGamma2);
      CLHEP::HepLorentzVector lv_rec = lv_gamma1 + lv_gamma2;

      CLHEP::Hep3Vector xGamma1(0, 0, 0);
      CLHEP::Hep3Vector xGamma2(0, 0, 0);

      const double mass = lv_rec.mag();
      CLHEP::HepLorentzVector fittedPi0Momentum;
      CLHEP::HepSymMatrix pi0ErrMatrix;
      TMatrixFSym Pi0ErrorTMatrix(4);

      MassFitKFit km;
      km.setMagneticField(1.5);
      km.addTrack(lv_gamma1, xGamma1, errorGamma1, 0);
      km.addTrack(lv_gamma2, xGamma2, errorGamma2, 0);
      const double MASS_PI0 = 0.1349739;

      km.setInvariantMass(MASS_PI0);

      unsigned err = km.doFit();
      double confLevel(DBL_MAX);
      if (!err) {
        confLevel = km.getCHIsq();
        fillFitted4Vector(km, fittedPi0Momentum, pi0ErrMatrix);
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j <= i ; j++) {
            Pi0ErrorTMatrix[i][j] = pi0ErrMatrix[i][j];
          }
        }


        if (pi0_mass_min < mass && mass < pi0_mass_max) {

          if (!Pi0Array) Pi0Array.create();
          Pi0Array.appendNew();
          m_Pi0Num = Pi0Array.getEntries() - 1;
          Pi0Array[m_Pi0Num]->setShowerId1(m_showerId1);
          Pi0Array[m_Pi0Num]->setShowerId2(m_showerId2);
          Pi0Array[m_Pi0Num]->setEnergy((float)fittedPi0Momentum.e());
          Pi0Array[m_Pi0Num]->setPx((float)fittedPi0Momentum.px());
          Pi0Array[m_Pi0Num]->setPy((float)fittedPi0Momentum.py());
          Pi0Array[m_Pi0Num]->setPz((float)fittedPi0Momentum.pz());

          Pi0Array[m_Pi0Num]->setMass((float)lv_rec.mag());
          Pi0Array[m_Pi0Num]->setMassFit((float)fittedPi0Momentum.m());
          Pi0Array[m_Pi0Num]->setChi2((float)confLevel);

          Pi0Array[m_Pi0Num]->setErrorMatrix(Pi0ErrorTMatrix);

          eclPi0ToGamma.add(m_Pi0Num, iGamma1);
          eclPi0ToGamma.add(m_Pi0Num, iGamma2);

        }//unfitted mass with pi0 mass window
      }//if MassFit not fail

    }//iGamma2
  }//iGamma1

  m_nEvent++;
}

void ECLPi0ReconstructorModule::endRun()
{
  m_nRun++;
}

void ECLPi0ReconstructorModule::terminate()
{
}


void ECLPi0ReconstructorModule::readErrorMatrix(int GammaIndex, CLHEP::HepSymMatrix& errorGamma1)
{

  StoreArray<ECLGamma> Gamma;
  ECLGamma* aECLGamma1 = Gamma[GammaIndex];
  TMatrixFSym GammaErrorMatrix(4);
  aECLGamma1-> getErrorMatrix(GammaErrorMatrix);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {

      errorGamma1[i][j] = GammaErrorMatrix[i][j];
    }
  }
  errorGamma1[4][4] = 1.;
  errorGamma1[5][5] = 1.;
  errorGamma1[6][6] = 1.;


}


/* Belle code
void ECLPi0ReconstructorModule::fit(CLHEP::HepLorentzVector m_shower1, CLHEP::HepLorentzVector m_shower2)
{

  const double MASS_PI0 = .1349739;
  const double SIGMA_Z = .0156;


  double mass = MASS_PI0;
  double sigma_z = SIGMA_Z;
  double low =  -0.0549739;
  double high =  0.0450261;
  region_type type = REGION_GEV;

  int m_iter_max = 5;
  double m_Df_limit = .1;
  double m_Dchi2_limit = .1;

  // covariant matrix
  CLHEP::HepMatrix V(6, 6, 0);

  V[0][0] = squ(errorE(m_shower1));
  V[0][1] = V[1][0] = errorE(m_shower1) * errorPhi(m_shower1) ;
  V[1][1] = squ(errorPhi(m_shower1));
  V[0][2] = V[2][0] = errorE(m_shower1) * errorTheta(m_shower1);
  V[1][2] = V[2][1] = errorPhi(m_shower1) * errorTheta(m_shower1);
  V[2][2] = squ(errorTheta(m_shower1));
  V[3][3] = squ(errorE(m_shower2));
  V[3][4] = V[4][3] = errorE(m_shower2) * errorPhi(m_shower2) ;
  V[4][4] = squ(errorPhi(m_shower2));
  V[3][5] = V[5][3] = errorE(m_shower2) * errorTheta(m_shower2);
  V[4][5] = V[5][4] = errorPhi(m_shower2) * errorTheta(m_shower2);
  V[5][5] = squ(errorTheta(m_shower2));


  V[2][5] = V[5][2] = squ(sigma_z)
                      * (sin(m_shower1.theta()) / cellR(m_shower1))
                      * (sin(m_shower2.theta()) / cellR(m_shower2));

  // initial measured parameters
  CLHEP::HepMatrix y0(6, 1);
  y0[0][0] = m_shower1.e();
  y0[1][0] = m_shower1.phi();
  y0[2][0] = m_shower1.theta();
  y0[3][0] = m_shower2.e();
  y0[4][0] = m_shower2.phi();
  y0[5][0] = m_shower2.theta();

  CLHEP::HepMatrix y(y0);
  CLHEP::HepMatrix Dy(6, 1, 0);

  int iter = 0;
  double Df = DBL_MAX;
  double f_old = DBL_MAX;
  double Dchi2 = DBL_MAX;
  double chi2_old = DBL_MAX;
  double mass_gg = DBL_MAX;
  double chi2 = DBL_MAX;
  bool exit_flag = false;
  while (1) {
    const double& E1 = y[0][0];
    const double& E2 = y[3][0];
    const double sin_theta1 = sin(y[2][0]);
    const double cos_theta1 = cos(y[2][0]);
    const double sin_theta2 = sin(y[5][0]);
    const double cos_theta2 = cos(y[5][0]);
    const double Dphi = y[1][0] - y[4][0];
    const double cos_Dphi = cos(Dphi);
    const double sin_Dphi = sin(Dphi);
    const double open_angle = sin_theta1 * sin_theta2 * cos_Dphi
                              + cos_theta1 * cos_theta2;
    const double mass2_gg = 2 * E1 * E2 * (1 - open_angle);
    mass_gg = (mass2_gg > 0) ? sqrt(mass2_gg) : -sqrt(-mass2_gg);

    if (exit_flag || ++iter > m_iter_max)
      break;

    // constraint
    CLHEP::HepMatrix f(1, 1);
    f[0][0] = mass2_gg - squ(mass);

    CLHEP::HepMatrix B(1, 6);
    B[0][0] = mass2_gg / E1;
    B[0][1] = 2 * E1 * E2 * sin_theta1 * sin_theta2 * sin_Dphi;
    B[0][2] = 2 * E1 * E2 * (-cos_theta1 * sin_theta2 * cos_Dphi
                             + sin_theta1 * cos_theta2);
    B[0][3] = mass2_gg / E2;
    B[0][4] = -B[0][1];
    B[0][5] = 2 * E1 * E2 * (-sin_theta1 * cos_theta2 * cos_Dphi
                             + cos_theta1 * sin_theta2);

    const double sigma2_mass2_gg = (B * V * B.T())[0][0];
//fix warning    const double sigma_mass2_gg
//fix warning      = (sigma2_mass2_gg > 0) ? sqrt(sigma2_mass2_gg) : -(-sigma2_mass2_gg);
//fix warning Poyuan    const double sigma_mass_gg = sigma_mass2_gg / mass_gg / 2;
    const double Dmass = mass_gg - mass;

//  const double sdev = Dmass / sigma_mass_gg;
//  change the sigma_mass_gg to a reasonable one
//      must be modified with correct evaluation of error matrx : 1999/5/28
    const double sdev = Dmass / .0055;
    if (type == REGION_SIGMA && (sdev < low || sdev > high)
        || type == REGION_GEV && (Dmass < low || Dmass > high))
      break;



    Dy = V * B.T() * (B * Dy - f) / sigma2_mass2_gg;
    y = y0 + Dy;
    int ierr;
    chi2 = (Dy.T() * V.inverse(ierr) * Dy)[0][0];
    Dchi2 = fabs(chi2 - chi2_old);
    chi2_old = chi2;
    Df = fabs(f[0][0] - f_old);
    f_old = f[0][0];
    if (Dchi2 < m_Dchi2_limit && Df < m_Df_limit)
      exit_flag = true;;
  }
  m_pi0E = y[0][0] + y[3][0];
  m_pi0px = y[0][0] * cos(y[1][0]) * sin(y[2][0])
            + y[3][0] * cos(y[4][0]) * sin(y[5][0]);
  m_pi0py = y[0][0] * sin(y[1][0]) * sin(y[2][0])
            + y[3][0] * sin(y[4][0]) * sin(y[5][0]);
  m_pi0pz = y[0][0] * cos(y[2][0]) + y[3][0] * cos(y[5][0]);
  m_pi0mass = mass_gg;

//  m_chi2 = chi2;  // protect...
  m_pi0chi2 = (chi2 > FLT_MAX) ? FLT_MAX : chi2;

}
*/
double ECLPi0ReconstructorModule::errorE(CLHEP::HepLorentzVector shower)
{
  double E = shower.e();
  double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
//sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%
  return sigmaE;

}
double ECLPi0ReconstructorModule::errorTheta(CLHEP::HepLorentzVector shower)
{

  double Theta = shower.theta();
  double Energy = shower.e();
  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(Energy) + 1.8 / sqrt(sqrt(Energy))) ;
//sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);


// /sw/belle/belle/latest/src/common/com-ecl/Ecl/TComEclParas.cc
  if (Theta < theta_f) {
    return sigmaX * squ(cos(Theta)) / zForward;
  } else if (Theta > theta_b) {
    return sigmaX * squ(cos(Theta)) / (-1 * zBackward);
  } else {
    return sigmaX * sin(Theta) / Rbarrel   ;
  }


}
double ECLPi0ReconstructorModule::errorPhi(CLHEP::HepLorentzVector shower)
{

  double Theta = shower.theta();
  double Energy = shower.e();
  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(Energy) + 1.8 / sqrt(sqrt(Energy))) ;
//sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);

// /sw/belle/belle/latest/src/common/com-ecl/Ecl/TComEclParas.cc
  if (Theta < theta_f) {
    return sigmaX / (zForward * tan(Theta))   ;
  } else if (Theta > theta_b) {
    return sigmaX / (zBackward * tan(Theta))   ;
  } else {
    return sigmaX / Rbarrel;
  }



}

/* Obtains the fitted 4-momentum (after mass constrained fit) */
unsigned ECLPi0ReconstructorModule::fillFitted4Vector(MassFitKFit& km, CLHEP::HepLorentzVector& fitMom, CLHEP::HepSymMatrix& covMatrix)
{
  unsigned n = km.getTrackCount();

  MakeMotherKFit kmm;
  kmm.setMagneticField(1.5);

  for (unsigned i = 0; i < n; ++i) {
    kmm.addTrack(km.getTrackMomentum(i),
                 km.getTrackPosition(i),
                 km.getTrackError(i),
                 km.getTrack(i).getCharge());

    for (unsigned j = i + 1; j < n; ++j) {
      kmm.setCorrelation(km.getCorrelation(i, j));
    }
  }

//  kmm.setVertex(km.getVertex());
//  kmm.setVertexError(km.getVertexError());

  unsigned err = kmm.doMake();
  if (err != 0)
    return 0;

  fitMom = kmm.getMotherMomentum();

  covMatrix = kmm.getMotherError();

  return 1;
}


double ECLPi0ReconstructorModule::cellR(CLHEP::HepLorentzVector shower)
{

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);

  double Theta = shower.theta();

  if (Theta < theta_f) {
    return  zForward / sin(Theta)   ;
  } else if (Theta > theta_b) {
    return  -1 * zBackward / sin(Theta)    ;
  } else {
    return  Rbarrel / sin(Theta) ;
  }


}

