/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower leakage, beam        *
 * backgrounds, and clustering                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Alon Hershenhorn (hersehn@physics.ubc.ca)                *
 *               Suman Koirala (suman@ntu.edu.tw)                         *
 *               Christopher Hearty (hearty@physics.ubc.ca)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//..This module
#include <ecl/modules/eclShowerCorrection/ECLShowerCorrectorModule.h>

//..Framework
#include <framework/logging/Logger.h>

//..ECL
#include <ecl/dbobjects/ECLLeakageCorrections.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLLeakagePosition.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCorrector)
REG_MODULE(ECLShowerCorrectorPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerCorrectorModule::ECLShowerCorrectorModule() : Module(),
  m_eclShowers(eclShowerArrayName()),
  m_eclLeakageCorrections("ECLLeakageCorrections")
{

  // Set description
  setDescription("ECLShowerCorrectorModule: Corrects energy of ECLShowers and highest energy crystal for shower leakage, beam backgrounds, and clustering");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerCorrectorModule::~ECLShowerCorrectorModule()
{
  ;
}

void ECLShowerCorrectorModule::initialize()
{
  B2DEBUG(28, "ECLShowerCorrectorModule::initialize()");

  //..Register in datastore
  m_eclShowers.registerInDataStore(eclShowerArrayName());

  //..Class to find cellID and position within crystal from theta and phi
  leakagePosition = new ECLLeakagePosition();

}

void ECLShowerCorrectorModule::beginRun()
{
  //-----------------------------------------------------------------
  //..Read in leakage corrections from database
  if (m_eclLeakageCorrections.hasChanged()) {

    //..Vectors of log(E) for each region
    std::vector<float> logEnergiesFwd = m_eclLeakageCorrections->getlogEnergiesFwd();
    std::vector<float> logEnergiesBrl = m_eclLeakageCorrections->getlogEnergiesBrl();
    std::vector<float> logEnergiesBwd = m_eclLeakageCorrections->getlogEnergiesBwd();

    //..Adjust the size of the vector of log energies to match the number in the payload
    nEnergies = logEnergiesBrl.size();
    B2INFO("ECLShowerCorrector beginRun: Number of energies = " << nEnergies);
    leakLogE.resize(nLeakReg, std::vector<float>(nEnergies, 0.));

    //..Copy values to leakLogE
    for (int ie = 0; ie < nEnergies; ie++) {
      leakLogE[0][ie] = logEnergiesFwd[ie];
      leakLogE[1][ie] = logEnergiesBrl[ie];
      leakLogE[2][ie] = logEnergiesBwd[ie];
    }

    //..Position and nCrys dependent corrections
    thetaCorrection = m_eclLeakageCorrections->getThetaCorrections();
    phiCorrection = m_eclLeakageCorrections->getPhiCorrections();
    crysCorrection = m_eclLeakageCorrections->getnCrystalCorrections();

    //..Relevant parameters
    nPositionBins = thetaCorrection.GetNbinsY();
    nCrysMax = crysCorrection.GetNbinsY();
    nXBins = nThetaID * nEnergies;
  }
}

void ECLShowerCorrectorModule::event()
{

  //-----------------------------------------------------------------
  //..Loop over all ECLShowers.
  for (auto& eclShower : m_eclShowers) {

    //..Only want to correct EM showers
    if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) {break;}

    //-----------------------------------------------------------------
    //..Shower quantities needed to find the correction.
    const double energyRaw = eclShower.getEnergy();
    const double energyRawHighest = eclShower.getEnergyHighestCrystal();

    //..Crystals used to calculate energy
    int nForEnergy = (int)(eclShower.getNumberOfCrystalsForEnergy() + 0.001);
    const int nCrys = std::min(nCrysMax, nForEnergy);

    //..Location starting point
    const int icellIDMaxE = eclShower.getCentralCellId();
    const float thetaLocation = eclShower.getTheta();
    const float phiLocation = eclShower.getPhi();

    //-----------------------------------------------------------------
    //..Location of shower. cellID = positionVector[0] is for debugging only
    std::vector<int> positionVector = leakagePosition->getLeakagePosition(icellIDMaxE, thetaLocation, phiLocation, nPositionBins);
    const int iThetaID = positionVector[1];
    const int iRegion = positionVector[2];
    const int iThetaBin = positionVector[3];
    const int iPhiBin = positionVector[4];
    const int iPhiMech = positionVector[5];

    //-----------------------------------------------------------------
    //..The correction is a function of corrected energy, so will need to iterate
    double correction = 0.96; // typical correction as starting point
    for (int iter = 0; iter < 2; iter++) {

      //..Energy points that bracket this value
      float logEnergy = log(energyRaw / correction);
      int ie0 = 0;
      if (logEnergy < leakLogE[iRegion][0]) {
        ie0 = 0;
      } else if (logEnergy > leakLogE[iRegion][nEnergies - 1]) {
        ie0 = nEnergies - 2;
      } else {
        while (logEnergy > leakLogE[iRegion][ie0 + 1]) {ie0++;}
      }

      //..Correction from lower energy point.
      //  The following include +1 because first histogram bin is 1 not 0.
      int iXBin = iThetaID + ie0 * nThetaID + 1; // thetaID / energy bin
      double thetaCor = thetaCorrection.GetBinContent(iXBin, iThetaBin + 1);
      double phiCor = phiCorrection.GetBinContent(iXBin + iPhiMech * nXBins, iPhiBin + 1);
      double crysCor = crysCorrection.GetBinContent(iXBin, nCrys + 1);
      const double cor0 = thetaCor * phiCor * crysCor;

      //..Correction from upper energy point
      iXBin = iThetaID + (ie0 + 1) * nThetaID + 1;
      thetaCor = thetaCorrection.GetBinContent(iXBin, iThetaBin + 1);
      phiCor = phiCorrection.GetBinContent(iXBin + iPhiMech * nXBins, iPhiBin + 1);
      crysCor = crysCorrection.GetBinContent(iXBin, nCrys + 1);
      const double cor1 = thetaCor * phiCor * crysCor;

      //..Interpolate (in logE)
      correction = cor0 + (cor1 - cor0) * (logEnergy - leakLogE[iRegion][ie0]) / (leakLogE[iRegion][ie0 + 1] - leakLogE[iRegion][ie0]);
    }

    //-----------------------------------------------------------------
    //..Apply correction
    const double correctedEnergy = energyRaw  / correction;
    const double correctedEnergyHighest = energyRawHighest / correction;
    B2DEBUG(28, "Correction factor=" << correction << ", correctedEnergy=" << correctedEnergy << ", correctedEnergyHighest=" <<
            correctedEnergyHighest);

    //..Set the corrected energies
    eclShower.setEnergy(correctedEnergy);
    eclShower.setEnergyHighestCrystal(correctedEnergyHighest);

  } // end loop over showers

}

void ECLShowerCorrectorModule::endRun()
{
  ;
}

void ECLShowerCorrectorModule::terminate()
{
  ;
}
