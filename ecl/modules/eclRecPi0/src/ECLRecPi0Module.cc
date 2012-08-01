/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecPi0/ECLRecPi0Module.h>
#include <ecl/dataobjects/MdstPi0.h>
#include <ecl/dataobjects/MdstGamma.h>

//#include <ecl/geometry/ECLGeometryPar.h>
//#include <ecl/rec_lib/TEclCFCR.h>
//#include <ecl/rec_lib/TRecEclCF.h>
//#include <ecl/rec_lib/TRecEclCFParameters.h>
//#include <ecl/rec_lib/TEclCFShower.h>
//#include <ecl/rec_lib/TRecEclCF.h>


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

#define PI 3.14159265358979323846

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLRecPi0)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLRecPi0Module::ECLRecPi0Module() : Module()
{
  //Set module properties
  setDescription("Creates Mdst_pi0 from Mdst_gamma.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //input
  addParam("MdstGammaInput", m_MdstGammaName,
           "//Input of this RecPi0  module", string("ECLGamma"));

  addParam("MdstPi0Output", m_MdstPi0Name,
           "//output of this RecPi0 module", string("ECLPi0"));

//  addParam("RandomSeed", m_randSeed, "User-supplied random seed; Default 0 for ctime", (unsigned int)(0));

}


ECLRecPi0Module::~ECLRecPi0Module()
{

}

void ECLRecPi0Module::initialize()
{

  gamma_energy_threshold = .02;
  opening_angle_cut_margin = M_PI / 180.; // 1 degree
  // pi0_mass_min = .134 - .0054 * 3, // 3 sigma
  // pi0_mass_max = .134 + .0054 * 3, // 3 sigma
  pi0_mass_min = 0.08; // Modify: 20040112 KM
  pi0_mass_max = 0.18; // Modify: 20040112 KM
  fit_flag = 1;
  chi2_max = 15.;
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void ECLRecPi0Module::beginRun()
{
}


void ECLRecPi0Module::event()
{

  StoreArray<MdstGamma> Gamma(m_MdstGammaName);

  if (!Gamma) {
    B2ERROR("Can not find ECLRecCRHits" << m_MdstGammaName << ".");
  }
  const int GNum = Gamma->GetEntriesFast();


  for (int iGamma = 0; iGamma < GNum - 1 ; iGamma++) {
    MdstGamma* aGamma = Gamma[iGamma];
    m_showerId1 = aGamma->getShowerId();
    m_px1 = aGamma->getpx();
    m_py1 = aGamma->getpy();
    m_pz1 = aGamma->getpz();
    CLHEP::Hep3Vector p3Gamma1(m_px1, m_py1, m_pz1);

    //cout<<"Pi0   "<<m_px1<<" "<<m_py1<<" "<<m_pz1<<" "<<" "<<sqrt(m_px1*m_px1+m_py1*m_py1+m_pz1*m_pz1)<<endl;

    // gamma energy cut
    const double EGamma1 = p3Gamma1.mag();
    if (EGamma1 < gamma_energy_threshold)continue;

    for (int jGamma = iGamma + 1; jGamma < GNum; jGamma++) {
      MdstGamma* aGamma2 = Gamma[jGamma];
      m_showerId2 = aGamma2->getShowerId();
      m_px2 = aGamma2->getpx();
      m_py2 = aGamma2->getpy();
      m_pz2 = aGamma2->getpz();

      CLHEP::Hep3Vector p3Gamma2(m_px2, m_py2, m_pz2);
      // gamma energy cut
      const double EGamma2 = p3Gamma2.mag();
      if (EGamma2 < gamma_energy_threshold)continue;
      CLHEP::Hep3Vector p3Rec = p3Gamma1 + p3Gamma2;


      CLHEP::HepLorentzVector lv_gamma1(p3Gamma1, EGamma1);
      CLHEP::HepLorentzVector lv_gamma2(p3Gamma2, EGamma2);
      CLHEP::HepLorentzVector lv_rec = lv_gamma1 + lv_gamma2;
      const double mass = lv_rec.mag();
      if (fit_flag) {
        fit(lv_gamma1, lv_gamma2);
        if (pi0_mass_min < mass && mass < pi0_mass_max) {

          StoreArray<MdstPi0> Pi0Array(m_MdstPi0Name);
          m_Pi0Num = Pi0Array->GetLast() + 1;
          new(Pi0Array->AddrAt(m_Pi0Num)) MdstPi0();
          Pi0Array[m_Pi0Num]->setShower1(aGamma->getShower());
          Pi0Array[m_Pi0Num]->setShower2(aGamma2->getShower());

          Pi0Array[m_Pi0Num]->setenergy((float)m_pi0E);
          Pi0Array[m_Pi0Num]->setpx((float)m_pi0px);
          Pi0Array[m_Pi0Num]->setpy((float)m_pi0py);
          Pi0Array[m_Pi0Num]->setpz((float)m_pi0pz);

          Pi0Array[m_Pi0Num]->setmass((float)lv_rec.mag());
          Pi0Array[m_Pi0Num]->setmassfit((float)m_pi0mass);
          Pi0Array[m_Pi0Num]->setchi2((float)m_pi0chi2);

          //cout << "Event " << m_nEvent << " Pi0 from Gamma " << m_showerId1 << " " << m_showerId2 << " " << m_pi0E << " " << m_pi0mass << endl;

        }
      } else if (pi0_mass_min < mass && mass < pi0_mass_max) {
      }



    }//gamma2
  }//gamma1

  m_nEvent++;
}

void ECLRecPi0Module::endRun()
{
  m_nRun++;
}

void ECLRecPi0Module::terminate()
{
}



void ECLRecPi0Module::fit(CLHEP::HepLorentzVector m_shower1, CLHEP::HepLorentzVector m_shower2)
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

double ECLRecPi0Module::errorE(CLHEP::HepLorentzVector shower)
{
  double E = shower.e();
  double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
//sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%
  return sigmaE;

}
double ECLRecPi0Module::errorTheta(CLHEP::HepLorentzVector shower)
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
double ECLRecPi0Module::errorPhi(CLHEP::HepLorentzVector shower)
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
double ECLRecPi0Module::cellR(CLHEP::HepLorentzVector shower)
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

