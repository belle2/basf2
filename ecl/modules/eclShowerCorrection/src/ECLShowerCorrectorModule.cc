/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//..This module
#include <ecl/modules/eclShowerCorrection/ECLShowerCorrectorModule.h>

//..Framework
#include <framework/logging/Logger.h>

//..ECL
#include <ecl/dbobjects/ECLLeakageCorrections.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLLeakagePosition.h>
#include <ecl/dataobjects/ECLElementNumbers.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCorrector);
REG_MODULE(ECLShowerCorrectorPureCsI);

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
  if (m_leakagePosition != nullptr)
    delete m_leakagePosition;
}

void ECLShowerCorrectorModule::initialize()
{
  B2DEBUG(28, "ECLShowerCorrectorModule::initialize()");

  //..Register in datastore
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eventLevelClusteringInfo.registerInDataStore();

  //..Class to find cellID and position within crystal from theta and phi
  m_leakagePosition = new ECLLeakagePosition();

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

    //..Position dependent corrections
    thetaCorrection = m_eclLeakageCorrections->getThetaCorrections();
    phiCorrection = m_eclLeakageCorrections->getPhiCorrections();

    //..Relevant parameters
    nPositionBins = thetaCorrection.GetNbinsY();
    nXBins = nThetaID * nEnergies;
  } else if (!m_eclLeakageCorrections.isValid()) {
    B2FATAL("ECLShowerCorrectorModule: missing eclLeakageCorrections payload");
  }

  //-----------------------------------------------------------------
  //..Get correction histograms related to the nOptimal number of crystals.
  //  All three have energy bin as y, crystals group number as x.
  //  Energy bin and group number for the shower are found in
  //  eclSplitterN1 and are stored in the ECLShower dataobject.

  if (m_eclNOptimal.hasChanged()) {

    //..Bias is the difference between the peak energy in nOptimal crystals
    //  before bias correction and the mc true deposited energy.
    m_bias = m_eclNOptimal->getBias();

    //..Log of the peak energy contained in nOptimal crystals after bias correction
    m_logPeakEnergy = m_eclNOptimal->getLogPeakEnergy();

    //..peakFracEnergy is the peak energy after subtracting the beam bias
    //  divided by the generated photon energy.
    m_peakFracEnergy = m_eclNOptimal->getPeakFracEnergy();
  } else if (!m_eclNOptimal.isValid()) {
    B2FATAL("ECLShowerCorrectorModule: missing eclNOptimal payload");
  }

  //-----------------------------------------------------------------
  //..Get correction histograms related to the nOptimal number of crystals.
  //  All three have energy bin as y, crystals group number as x.
  //  Energy bin and group number for the shower are found in
  //  eclSplitterN1 and are stored in the ECLShower dataobject.

  if (m_eclNOptimal.hasChanged()) {

    //..Bias is the difference between the peak energy in nOptimal crystals
    //  before bias correction and the mc true deposited energy.
    m_bias = m_eclNOptimal->getBias();

    //..Log of the peak energy contained in nOptimal crystals after bias correction
    m_logPeakEnergy = m_eclNOptimal->getLogPeakEnergy();

    //..peakFracEnergy is the peak energy after subtracting the beam bias
    //  divided by the generated photon energy.
    m_peakFracEnergy = m_eclNOptimal->getPeakFracEnergy();
  }
}

void ECLShowerCorrectorModule::event()
{

  //-----------------------------------------------------------------
  //..Loop over all ECLShowers.
  for (auto& eclShower : m_eclShowers) {

    //..Only want to correct EM showers
    if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) {break;}

    //..Will correct both raw cluster energy and energy of the center crystal
    const double energyRaw = eclShower.getEnergy();
    const double energyRawHighest = eclShower.getEnergyHighestCrystal();

    //-----------------------------------------------------------------
    //..Correct for bias and peak value corresponding to nOptimal crystals
    const int iGroup = eclShower.getNOptimalGroupIndex();
    const int iEnergy = eclShower.getNOptimalEnergyBin();
    const double e3x3 = eclShower.getNOptimalEnergy(); // only for debugging

    //..The optimal number of crystals for generated energy iEnergy and
    //  group of crystals iGroup is nOptimal. There are three corresponding bins for
    //  each of the 2D correction histograms m_logPeakEnergy, m_bias, and
    //  m_peakFracEnergy. For example, the bias for nOptimal crystals in group iGroup is
    //  m_bias(3*iGroup+1, iEnergy+1) for generated energy point iEnergy,
    //  m_bias(3*iGroup+2, iEnergy+1) for generated energy point iEnergy-1, and
    //  m_bias(3*iGroup+3, iEnergy+1) for generated energy point iEnergy+1
    //  This allows the correction to be interpolated to an arbitrary observed energy.
    const int iy = iEnergy + 1;

    const int ixNom = 3 * iGroup + 1;
    const int ixLowerE = ixNom + 1;
    const int ixHigherE = ixNom + 2;

    const double logENom = m_logPeakEnergy.GetBinContent(ixNom, iy);
    const double logELower = m_logPeakEnergy.GetBinContent(ixLowerE, iy);
    const double logEHigher = m_logPeakEnergy.GetBinContent(ixHigherE, iy);

    const double biasNom = m_bias.GetBinContent(ixNom, iy);
    const double biasLower = m_bias.GetBinContent(ixLowerE, iy);
    const double biasHigher = m_bias.GetBinContent(ixHigherE, iy);

    const double peakNom = m_peakFracEnergy.GetBinContent(ixNom, iy);
    const double peakLower = m_peakFracEnergy.GetBinContent(ixLowerE, iy);
    const double peakHigher = m_peakFracEnergy.GetBinContent(ixHigherE, iy);

    //..Interpolate in log of raw energy
    const double logESumN = log(energyRaw);

    double logEOther = logELower;
    double biasOther = biasLower;
    double peakOther = peakLower;
    if (logESumN > logENom) {
      logEOther = logEHigher;
      biasOther = biasHigher;
      peakOther = peakHigher;
    }

    //..The nominal and "other" energies may be identical if this is the first or last energy
    double bias = biasNom;
    double peak = peakNom;
    if (std::abs(logEOther - logENom) > 0.0001) {
      bias = biasNom + (biasOther - biasNom) * (logESumN - logENom) / (logEOther - logENom);
      peak = peakNom + (peakOther - peakNom) * (logESumN - logENom) / (logEOther - logENom);
    }

    //..Correct raw energy for bias and peak
    const double ePartialCorr = (energyRaw - bias) / peak;

    //-----------------------------------------------------------------
    //..Shower quantities needed to find the correction.

    //..Location starting point
    const int icellIDMaxE = eclShower.getCentralCellId();
    const float thetaLocation = eclShower.getTheta();
    const float phiLocation = eclShower.getPhi();

    //-----------------------------------------------------------------
    //..Location of shower. cellID = positionVector[0] is for debugging only
    std::vector<int> positionVector = m_leakagePosition->getLeakagePosition(icellIDMaxE, thetaLocation, phiLocation, nPositionBins);
    const int iThetaID = positionVector[1];
    const int iRegion = positionVector[2];
    const int iThetaBin = positionVector[3];
    const int iPhiBin = positionVector[4];
    const int iPhiMech = positionVector[5];

    //-----------------------------------------------------------------
    //..Energy points that bracket this value
    float logEnergy = log(ePartialCorr);
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
    const double cor0 = thetaCor * phiCor;

    //..Correction from upper energy point
    iXBin = iThetaID + (ie0 + 1) * nThetaID + 1;
    thetaCor = thetaCorrection.GetBinContent(iXBin, iThetaBin + 1);
    phiCor = phiCorrection.GetBinContent(iXBin + iPhiMech * nXBins, iPhiBin + 1);
    const double cor1 = thetaCor * phiCor;

    //..Interpolate (in logE)
    const double positionCor = cor0 + (cor1 - cor0) * (logEnergy - leakLogE[iRegion][ie0]) / (leakLogE[iRegion][ie0 + 1] -
                               leakLogE[iRegion][ie0]);

    //-----------------------------------------------------------------
    //..Apply correction. Assume the same correction for the maximum energy crystal.
    const double correctedEnergy = ePartialCorr  / positionCor;
    const double overallCorrection = energyRaw / correctedEnergy;
    const double correctedEnergyHighest = energyRawHighest / overallCorrection;

    //..Set the corrected energies
    eclShower.setEnergy(correctedEnergy);
    eclShower.setEnergyHighestCrystal(correctedEnergyHighest);

    B2DEBUG(28, "ECLShowerCorrectorModule: cellID: " << positionVector[0] << " iG: " << iGroup << " iE: " << iEnergy << " Eraw: " <<
            energyRaw << " E3x3: " << e3x3 << " Ecor: " << correctedEnergy);
    B2DEBUG(28, " peakNom: " << peakNom << " biasNom: " << biasNom << " positionCor: " << positionCor << " overallCor: " <<
            overallCorrection);

  } // end loop over showers

  //-----------------------------------------------------------------
  //..Count number of showers in each region for EventLevelClusteringInfo
  uint16_t nShowersPerRegion[nLeakReg] = {};
  for (auto& eclShower : m_eclShowers) {
    if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) {break;}
    const int iCellId = eclShower.getCentralCellId();
    if (ECLElementNumbers::isForward(iCellId)) {nShowersPerRegion[0]++;}
    if (ECLElementNumbers::isBarrel(iCellId)) {nShowersPerRegion[1]++;}
    if (ECLElementNumbers::isBackward(iCellId)) {nShowersPerRegion[2]++;}
  }

  //..Store
  m_eventLevelClusteringInfo->setNECLShowersFWD(nShowersPerRegion[0]);
  m_eventLevelClusteringInfo->setNECLShowersBarrel(nShowersPerRegion[1]);
  m_eventLevelClusteringInfo->setNECLShowersBWD(nShowersPerRegion[2]);

}

void ECLShowerCorrectorModule::endRun()
{
  ;
}

void ECLShowerCorrectorModule::terminate()
{
  ;
}
