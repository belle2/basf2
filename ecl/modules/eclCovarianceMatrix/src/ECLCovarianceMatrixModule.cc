/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates the covariance matrix for photon showers.       *
 * The matrix depends on the shower region (FWD, Bartel, BWD)             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclCovarianceMatrix/ECLCovarianceMatrixModule.h>

// ROOT
#include "TMath.h"

// MDST
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>

// NAMESPACES
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLCovarianceMatrix)
REG_MODULE(ECLCovarianceMatrixPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLCovarianceMatrixModule::ECLCovarianceMatrixModule() : Module(),
  m_eclShowers(eclShowerArrayName()),
  m_eventLevelClusteringInfo(eventLevelClusteringInfoName())
{
  // Set description
  setDescription("ECLCovarianceMatrix: Sets the ECL photon shower covariance matrix.");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLCovarianceMatrixModule::~ECLCovarianceMatrixModule()
{
}

void ECLCovarianceMatrixModule::initialize()
{
  // Register in datastore
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eventLevelClusteringInfo.registerInDataStore(eventLevelClusteringInfoName());
}

void ECLCovarianceMatrixModule::beginRun()
{
  // TODO: callback
  ;
}

void ECLCovarianceMatrixModule::event()
{

  // Get the event background level
  const int bkgdcount = m_eventLevelClusteringInfo->getNECLCalDigitsOutOfTime();
  double background = 0.0; // from out of time digit counting
  if (m_fullBkgdCount > 0) {
    background = static_cast<double>(bkgdcount) / m_fullBkgdCount;
  }

  // loop over all ECLShowers
  for (auto& eclShower : m_eclShowers) {

    // Only correct for photon showers and high energey electrons
    if (eclShower.getHypothesisId() == ECLCluster::c_nPhotons) {

      const double energy = eclShower.getEnergy();

      // 1/energy
      double invEnergy = 0;
      if (energy > 0.) invEnergy = 1. / energy;

      //1/energy^0.5
      double invRoot2Energy = 0;
      if (energy > 0.) invRoot2Energy = 1. / TMath::Power(energy, 0.5);

      //1/energy^0.25
      double invRoot4Energy = 0;
      if (energy > 0.) invRoot4Energy = 1. / TMath::Power(energy, 0.25);

      int detregion = eclShower.getDetectorRegion(); // FWD: 1, Barrel: 2, BWD: 3
      if (detregion == 11 or detregion == 13) detregion = 2;

      double sigmaEnergy = 0.;
      double sigmaTheta = 0.;
      double sigmaPhi = 0.;

      // three background levels, three detector regions, energy, theta and phi (needs to be revisited soon!)
      if (detregion == 1 and background < 0.05) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.0449188); }
        else {sigmaEnergy = energy * (-0.0912379 * invEnergy + 1.91849 * invRoot2Energy + -2.82169 * invRoot4Energy + 3.03119) / 100.;}
      } else if (detregion == 1 and background <= 0.3) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.100769); }
        else {sigmaEnergy = energy * (-0.0725928 * invEnergy + 3.4685 * invRoot2Energy + -5.44127 * invRoot4Energy + 4.12045) / 100.;}
      } else if (detregion == 1 and background > 0.3) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.148462); }
        else {sigmaEnergy = energy * (-0.239931 * invEnergy + 6.94958 * invRoot2Energy + -10.4085 * invRoot4Energy + 5.92412) / 100.;}
      }

      if (detregion == 1 and background < 0.05) {
        if (energy <= 0.0289609) { sigmaTheta = 1e-3 * (6.4008);}
        else {sigmaTheta = 1e-3 * (-0.110397 * invEnergy + 0.753603 * invRoot2Energy + 2.63652 * invRoot4Energy + -0.606703);}
      } else if (detregion == 1 and background <= 0.3) {
        if (energy <= 0.0274562) { sigmaTheta = 1e-3 * (7.39868);}
        else {sigmaTheta = 1e-3 * (-0.207278 * invEnergy + 2.68616 * invRoot2Energy + -0.905487 * invRoot4Energy + 0.961485);}
      } else if (detregion == 1 and background > 0.3) {
        if (energy <= 0.022) { sigmaTheta = 1e-3 * (8.83505);}
        else {sigmaTheta = 1e-3 * (-0.160921 * invEnergy + 2.35311 * invRoot2Energy + -0.107975 * invRoot4Energy + 0.565367);}
      }

      if (detregion == 1 and background < 0.05) {
        if (energy <= 0.0391279) {sigmaPhi = 1e-3 * (15.4147);}
        else {sigmaPhi = 1e-3 * (-0.35934 * invEnergy + 2.01807 * invRoot2Energy + 7.26313 * invRoot4Energy + -1.93438);}
      } else if (detregion == 1 and background <= 0.3) {
        if (energy <= 0.022) {sigmaPhi = 1e-3 * (18.1096);}
        else {sigmaPhi = 1e-3 * (-0.178258 * invEnergy + 1.39155 * invRoot2Energy + 6.97462 * invRoot4Energy + -1.2795);}
      } else if (detregion == 1 and background > 0.3) {
        if (energy <= 0.0224565) {sigmaPhi = 1e-3 * (20.1605);}
        else {sigmaPhi = 1e-3 * (-0.344617 * invEnergy + 3.80823 * invRoot2Energy + 4.08729 * invRoot4Energy + -0.464714);}
      }

      if (detregion == 2 and background < 0.05) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.0466552); }
        else {sigmaEnergy = energy * (-0.0473491 * invEnergy + 1.02015 * invRoot2Energy + -0.705164 * invRoot4Energy + 1.77086) / 100.;}
      } else if (detregion == 2 and background <= 0.3) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.112299); }
        else {sigmaEnergy = energy * (0.0469588 * invEnergy + 1.81959 * invRoot2Energy + -2.13666 * invRoot4Energy + 2.37568) / 100.;}
      } else if (detregion == 2 and background > 0.3) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.164881); }
        else {sigmaEnergy = energy * (-0.267069 * invEnergy + 7.6176 * invRoot2Energy + -11.0341 * invRoot4Energy + 5.9203) / 100.;}
      }

      if (detregion == 2 and background < 0.05) {
        if (energy <= 0.0309077) { sigmaTheta = 1e-3 * (9.31098);}
        else {sigmaTheta = 1e-3 * (-0.221073 * invEnergy + 2.19876 * invRoot2Energy + 1.50833 * invRoot4Energy + 0.359609);}
      } else if (detregion == 2 and background <= 0.3) {
        if (energy <= 0.022) { sigmaTheta = 1e-3 * (11.6166);}
        else {sigmaTheta = 1e-3 * (-0.126603 * invEnergy + 1.81898 * invRoot2Energy + 1.85132 * invRoot4Energy + 0.300728);}
      } else if (detregion == 2 and background > 0.3) {
        if (energy <= 0.022) { sigmaTheta = 1e-3 * (13.159);}
        else {sigmaTheta = 1e-3 * (-0.316561 * invEnergy + 4.94686 * invRoot2Energy + -3.12199 * invRoot4Energy + 2.30275);}
      }

      if (detregion == 2 and background < 0.05) {
        if (energy <= 0.0381178) {sigmaPhi = 1e-3 * (10.7661);}
        else {sigmaPhi = 1e-3 * (-0.21842 * invEnergy + 0.648976 * invRoot2Energy + 7.1901 * invRoot4Energy + -3.10025);}
      } else if (detregion == 2 and background <= 0.3) {
        if (energy <= 0.022237) {sigmaPhi = 1e-3 * (12.6804);}
        else {sigmaPhi = 1e-3 * (-0.141507 * invEnergy + 0.488293 * invRoot2Energy + 7.30055 * invRoot4Energy + -3.13591);}
      } else if (detregion == 2 and background > 0.3) {
        if (energy <= 0.024905) {sigmaPhi = 1e-3 * (14.8726);}
        else {sigmaPhi = 1e-3 * (-0.323752 * invEnergy + 3.60933 * invRoot2Energy + 2.48526 * invRoot4Energy + -1.25493);}
      }

      if (detregion == 3 and background < 0.05) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.0466662); }
        else {sigmaEnergy = energy * (-0.106729 * invEnergy + 2.38163 * invRoot2Energy + -4.11299 * invRoot4Energy + 4.14056) / 100.;}
      } else if (detregion == 3 and background <= 0.3) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.128851); }
        else {sigmaEnergy = energy * (-0.204362 * invEnergy + 6.43 * invRoot2Energy + -10.8673 * invRoot4Energy + 7.04059) / 100.;}
      } else if (detregion == 3 and background > 0.3) {
        if (energy <= 0.022) {sigmaEnergy = energy * (0.255153); }
        else {sigmaEnergy = energy * (-0.316245 * invEnergy + 11.5211 * invRoot2Energy + -18.3626 * invRoot4Energy + 9.89422) / 100.;}
      }

      if (detregion == 3 and background < 0.05) {
        if (energy <= 0.0318079) { sigmaTheta = 1e-3 * (10.4446);}
        else {sigmaTheta = 1e-3 * (-0.295827 * invEnergy + 3.37748 * invRoot2Energy + -0.284446 * invRoot4Energy + 1.48098);}
      } else if (detregion == 3 and background <= 0.3) {
        if (energy <= 0.022) { sigmaTheta = 1e-3 * (13.828);}
        else {sigmaTheta = 1e-3 * (-0.252479 * invEnergy + 4.18833 * invRoot2Energy + -2.04822 * invRoot4Energy + 2.38486);}
      } else if (detregion == 3 and background > 0.3) {
        if (energy <= 0.022) { sigmaTheta = 1e-3 * (17.0354);}
        else {sigmaTheta = 1e-3 * (-0.420215 * invEnergy + 7.08904 * invRoot2Energy + -5.88269 * invRoot4Energy + 3.61643);}
      }

      if (detregion == 3 and background < 0.05) {
        if (energy <= 0.022) {sigmaPhi = 1e-3 * (15.129);}
        else {sigmaPhi = 1e-3 * (-0.0534441 * invEnergy + -1.72466 * invRoot2Energy + 12.8625 * invRoot4Energy + -4.21203);}
      } else if (detregion == 3 and background <= 0.3) {
        if (energy <= 0.022) {sigmaPhi = 1e-3 * (20.1408);}
        else {sigmaPhi = 1e-3 * (0.0869293 * invEnergy + -1.93352 * invRoot2Energy + 12.8105 * invRoot4Energy + -4.03756);}
      } else if (detregion == 3 and background > 0.3) {
        if (energy <= 0.022) {sigmaPhi = 1e-3 * (23.2771);}
        else {sigmaPhi = 1e-3 * (-0.439611 * invEnergy + 5.74196 * invRoot2Energy + 1.76693 * invRoot4Energy + -0.0407866);}
      }

      B2DEBUG(175, "energy=" << energy << ", detector region=" << detregion);
      B2DEBUG(175, "sigmaEnergy=" << sigmaEnergy << ", sigmaPhi=" << sigmaPhi << ", sigmaTheta=" << sigmaTheta);

      double covMatrix[6] = {sigmaEnergy * sigmaEnergy, 0.0, sigmaPhi * sigmaPhi, 0.0, 0.0, sigmaTheta * sigmaTheta};
      eclShower.setCovarianceMatrix(covMatrix);
    }
  }
}

void ECLCovarianceMatrixModule::endRun()
{
  ;
}

void ECLCovarianceMatrixModule::terminate()
{
  ;
}
