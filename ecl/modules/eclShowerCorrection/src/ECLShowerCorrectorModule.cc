/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the energy correction for EM shower (mainly       *
 * longitudinal leakage): corr = raw * correctionfactor.                  *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Alon Hershenhorn (hershen@physics.ubc.ca) (AH)           *
 *               Suman Koirala (Suman Koirala <suman@ntu.edu.tw>) (SK)    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerCorrection/ECLShowerCorrectorModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/database/DBArray.h>

// ECL
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>

// ROOT
#include <TMath.h>

// OTHER
#include <vector>
#include <fstream>      // std::ifstream

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCorrector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerCorrectorModule::ECLShowerCorrectorModule() : Module(),
  m_leakageCorrectionPtr("ecl_shower_corrector_leakage_corrections"),
  m_eclShowers(eclShowerArrayName()),
  m_eclEventInformation(eclEventInformationName())
{

  // Set description
  setDescription("ECLShowerCorrectorModule: Corrects for MC truth to reconstruction shower and highest crystal energy differences");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerCorrectorModule::~ECLShowerCorrectorModule()
{
  ;
}

void ECLShowerCorrectorModule::initialize()
{
  B2DEBUG(175, "ECLShowerCorrectorModule::initialize()");

  // Register in datastore
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclEventInformation.registerInDataStore(eclEventInformationName());
}

void ECLShowerCorrectorModule::beginRun()
{
  // TODO: callback!
  prepareLeakageCorrections();
}

void ECLShowerCorrectorModule::event()
{

  // Get the event background level.
  const int bkgdcount = m_eclEventInformation->getBackgroundECL();
  double backgroundLevel = 0.0; // from out of time digit counting
  if (m_fullBkgdCount > 0) {
    backgroundLevel = static_cast<double>(bkgdcount) / m_fullBkgdCount;
  }

  // Loop over all ECLShowers.
  for (auto& eclShower : m_eclShowers) {

    // Only correct N1 showers! N2 showers keep the raw energy! (TF)
    if (eclShower.getHypothesisId() == ECLConnectedRegion::c_N1) {

      const double energy        = eclShower.getEnergy();
      const double energyHighest = eclShower.getEnergyHighestCrystal();
      const double theta         = eclShower.getTheta() * TMath::RadToDeg();
      const double phi           = eclShower.getPhi() * TMath::RadToDeg();

      // Get the correction
      double correctionFactor = getLeakageCorrection(theta, phi, energy, backgroundLevel);
      B2DEBUG(175, "theta=" << theta << ", phi=" << phi << ", E=" << energy << ", BG=" << backgroundLevel << " --> correction factor=" <<
              correctionFactor);

      if (correctionFactor < 1.e-5 or correctionFactor > 10.) {
        B2ERROR("correction factor=" << correctionFactor << " is very small/too large! Resetting to 1.0.");
        correctionFactor = 1.0;
      }

      const double correctedEnergy = energy * correctionFactor;
      const double correctedEnergyHighest = energyHighest * correctionFactor;
      B2DEBUG(175, "correction factor=" << correctionFactor << ", correctedEnergy=" << correctedEnergy << ", correctedEnergyHighest=" <<
              correctedEnergyHighest);

      // Set the correction
      eclShower.setEnergy(correctedEnergy);
      eclShower.setEnergyHighestCrystal(correctedEnergyHighest);

    } // end correction N1 only
  } // end loop over all shower

}

void ECLShowerCorrectorModule::endRun()
{
  ;
}

void ECLShowerCorrectorModule::terminate()
{
  ;
}

void ECLShowerCorrectorModule::prepareLeakageCorrections()
{
  // Prepare energy correction constants taken from the database to be used in an interpolation correction
  // get all information from the payload
  m_numOfBfBins        = m_leakageCorrectionPtr->getNumOfBfBins()[0];
  m_numOfEnergyBins    = m_leakageCorrectionPtr->getNumOfEnergyBins()[0];
  m_numOfPhiBins       = m_leakageCorrectionPtr->getNumOfPhiBins()[0];
  m_numOfReg1ThetaBins = m_leakageCorrectionPtr->getNumOfReg1ThetaBins()[0];
  m_numOfReg2ThetaBins = m_leakageCorrectionPtr->getNumOfReg2ThetaBins()[0];
  m_numOfReg3ThetaBins = m_leakageCorrectionPtr->getNumOfReg3ThetaBins()[0];

  // Resize the multidimensional vectors
  m_reg1CorrFactorArrays.resize(m_numOfBfBins);
  m_reg2CorrFactorArrays.resize(m_numOfBfBins);
  m_reg3CorrFactorArrays.resize(m_numOfBfBins);

  for (int iBfBin = 0; iBfBin < m_numOfBfBins; iBfBin++) {
    m_reg1CorrFactorArrays[iBfBin].resize(m_numOfEnergyBins);
    m_reg2CorrFactorArrays[iBfBin].resize(m_numOfEnergyBins);
    m_reg3CorrFactorArrays[iBfBin].resize(m_numOfEnergyBins);

    for (int iEBin = 0; iEBin < m_numOfEnergyBins; iEBin++) {
      m_reg1CorrFactorArrays[iBfBin][iEBin].resize(m_numOfPhiBins);
      m_reg2CorrFactorArrays[iBfBin][iEBin].resize(m_numOfPhiBins);
      m_reg3CorrFactorArrays[iBfBin][iEBin].resize(m_numOfPhiBins);

      for (int iPhiBin = 0; iPhiBin < m_numOfPhiBins; iPhiBin++) {
        m_reg1CorrFactorArrays[iBfBin][iEBin][iPhiBin].resize(m_numOfReg1ThetaBins);
        m_reg2CorrFactorArrays[iBfBin][iEBin][iPhiBin].resize(m_numOfReg2ThetaBins);
        m_reg3CorrFactorArrays[iBfBin][iEBin][iPhiBin].resize(m_numOfReg3ThetaBins);
      }
    }
  }

  m_avgRecEn = m_leakageCorrectionPtr->getAvgRecEn();

  // Fill the multidimensional vectors
  m_correctionFactor = m_leakageCorrectionPtr->getCorrectionFactor();
  m_bgFractionBinNum = m_leakageCorrectionPtr->getBgFractionBinNum();
  m_regNum = m_leakageCorrectionPtr->getRegNum();
  m_phiBinNum = m_leakageCorrectionPtr->getPhiBinNum();
  m_thetaBinNum = m_leakageCorrectionPtr->getThetaBinNum();
  m_energyBinNum = m_leakageCorrectionPtr->getEnergyBinNum();
  m_correctionFactor = m_leakageCorrectionPtr->getCorrectionFactor();

  for (unsigned i = 0; i < m_correctionFactor.size(); ++i) {
    if (m_regNum[i] == 1) {
      m_reg1CorrFactorArrays[m_bgFractionBinNum[i]][m_energyBinNum[i]][m_phiBinNum[i]][m_thetaBinNum[i]] = m_correctionFactor[i];
    } else if (m_regNum[i] == 2) {
      m_reg2CorrFactorArrays[m_bgFractionBinNum[i]][m_energyBinNum[i]][m_phiBinNum[i]][m_thetaBinNum[i]] = m_correctionFactor[i];
    } else if (m_regNum[i] == 3) {
      m_reg3CorrFactorArrays[m_bgFractionBinNum[i]][m_energyBinNum[i]][m_phiBinNum[i]][m_thetaBinNum[i]] = m_correctionFactor[i];
    }
  }

  m_phiPeriodicity = m_leakageCorrectionPtr->getPhiPeriodicity()[0];
  m_lReg1Theta = m_leakageCorrectionPtr->getLReg1Theta()[0];
  m_hReg1Theta = m_leakageCorrectionPtr->getHReg1Theta()[0];
  m_lReg2Theta = m_leakageCorrectionPtr->getLReg2Theta()[0];
  m_hReg2Theta = m_leakageCorrectionPtr->getHReg2Theta()[0];
  m_lReg3Theta = m_leakageCorrectionPtr->getLReg3Theta()[0];
  m_hReg3Theta = m_leakageCorrectionPtr->getHReg3Theta()[0];

}


double ECLShowerCorrectorModule::getLeakageCorrection(const double theta,
                                                      const double phi,
                                                      const double energy,
                                                      const double background) const
{
  // Corrections are available for BGx0 and BGx1.0 (12th Campaign) only.
  int bf = 0;
  if (background > 0.1) bf = 1; // If users choose values other than BGx0 and BGx1 corrections will not be optimal.

  // Correction factor (multiplicative).
  double result = 1.0;
  double x = 0.0;
  double x0 = 0.0;
  double  x1 = 0.0;
  double xd = 0.0;

  // Convert -180..180 to 0..360
  const double phiMod = phi + 180;

  int x0Bin = 0;
  int x1Bin = m_numOfPhiBins - 1;
  int phiGap = 0;
  phiGap = int(phiMod / (360.0 / (m_phiPeriodicity * 1.0)));
  x = phiMod - phiGap * (360.0 / (m_phiPeriodicity * 1.0));

  const double phiBinWidth = 360.0 / (1.0 * m_numOfPhiBins * m_phiPeriodicity);

  if (x <= phiBinWidth / 2.0) {
    x0 = 0.00;
    x1 = phiBinWidth / 2.0;
    x0Bin = 0;
    x1Bin = m_numOfPhiBins - 1;
  }

  for (int ii = 0; ii < (m_numOfPhiBins - 1); ii++) {
    if (x >= (ii * phiBinWidth + phiBinWidth / 2.0) and x < ((ii + 1)*phiBinWidth + phiBinWidth / 2.0)) {
      x0 = (ii * phiBinWidth + phiBinWidth / 2.0);
      x1 = ((ii + 1) * phiBinWidth + phiBinWidth / 2.0);
      x0Bin = ii;
      x1Bin = ii + 1;
    }
  }

  if (x >= phiBinWidth * m_numOfPhiBins - phiBinWidth / 2.0) {
    x0 = phiBinWidth * m_numOfPhiBins - phiBinWidth / 2.0;
    x1 = phiBinWidth * m_numOfPhiBins;
    x0Bin = m_numOfPhiBins - 1;
    x1Bin = 0;
  }

  B2DEBUG(175, "m_numOfPhiBins=" << m_numOfPhiBins << " x0Bin=" << x0Bin << " x1Bin=" << x1Bin);

  double y = theta;
  double y0 = 0.;
  double y1 = 0.;
  double yd = 0.;
  int y0Bin = 0;;
  int y1Bin = 0;

  if (theta >= m_lReg1Theta and theta <= m_hReg1Theta) {
    int reg1thetaBin = 0;
    reg1thetaBin = int((theta - m_lReg1Theta) * m_numOfReg1ThetaBins / (m_hReg1Theta - m_lReg1Theta));
    y = theta;
    if (y < m_lReg1Theta + 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins))) {
      y0 = m_lReg1Theta;
      y1 = m_lReg1Theta + 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins));
      y0Bin = 0;
      y1Bin = 0;
    } else if (y >= m_lReg1Theta + 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins))
               and y < m_hReg1Theta - 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins))) {
      y0 = m_lReg1Theta + 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins)) + reg1thetaBin * ((
             m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins));
      y1 = m_lReg1Theta + 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins)) + (reg1thetaBin + 1) * ((
             m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins));
      y0Bin = reg1thetaBin;
      y1Bin = reg1thetaBin + 1;
      if (y0Bin == m_numOfReg1ThetaBins - 1) y1Bin = m_numOfReg1ThetaBins - 1;
    } else if (y >= m_hReg1Theta - 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins))) {
      y0 = m_hReg1Theta - 0.5 * ((m_hReg1Theta - m_lReg1Theta) / (1.0 * m_numOfReg1ThetaBins));
      y1 = m_hReg1Theta;
      y0Bin = m_numOfReg1ThetaBins - 1;
      y1Bin = m_numOfReg1ThetaBins - 1;
    } else {
      y1 = m_hReg1Theta;
      y0Bin = m_numOfReg1ThetaBins - 1;
      y1Bin = m_numOfReg1ThetaBins - 1;
    }
  } //End of region 1
  else if (theta >= m_lReg2Theta && theta <= m_hReg2Theta) {
    int reg2thetaBin = 0;
    reg2thetaBin = int((theta - m_lReg2Theta) * m_numOfReg2ThetaBins / (m_hReg2Theta - m_lReg2Theta));
    y = theta;
    if (y < m_lReg2Theta + 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins))) {
      y0 = m_lReg2Theta;
      y1 = m_lReg2Theta + 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins));
      y0Bin = 0;
      y1Bin = 0;
    } else if (y >= m_lReg2Theta + 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins))
               and y < m_hReg2Theta - 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins))) {
      y0 = m_lReg2Theta + 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins)) + reg2thetaBin * ((
             m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins));
      y1 = m_lReg2Theta + 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins)) + (reg2thetaBin + 1) * ((
             m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins));
      y0Bin = reg2thetaBin;
      y1Bin = reg2thetaBin + 1;
      if (y0Bin == m_numOfReg2ThetaBins - 1) y1Bin = m_numOfReg2ThetaBins - 1;
    } else if (y >= m_hReg2Theta - 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins))) {
      y0 = m_hReg2Theta - 0.5 * ((m_hReg2Theta - m_lReg2Theta) / (1.0 * m_numOfReg2ThetaBins)); y1 = m_hReg2Theta;
      y0Bin = m_numOfReg2ThetaBins - 1; y1Bin = m_numOfReg2ThetaBins - 1;
    } else {y1 = m_hReg2Theta; y0Bin = m_numOfReg2ThetaBins - 1; y1Bin = m_numOfReg2ThetaBins - 1;}
  } //End of region 2
  else if (theta >= m_lReg3Theta && theta <= m_hReg3Theta) {
    int reg3thetaBin = 0;
    reg3thetaBin = int((theta - m_lReg3Theta) * m_numOfReg3ThetaBins / (m_hReg3Theta - m_lReg3Theta));
    y = theta;
    if (y < m_lReg3Theta + 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins))) {
      y0 = m_lReg3Theta;
      y1 = m_lReg3Theta + 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins));
      y0Bin = 0;
      y1Bin = 0;
    } else if (y >= m_lReg3Theta + 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins))
               and y < m_hReg3Theta - 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins))) {
      y0 = m_lReg3Theta + 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins)) + reg3thetaBin * ((
             m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins));
      y1 = m_lReg3Theta + 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins)) + (reg3thetaBin + 1) * ((
             m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins));
      y0Bin = reg3thetaBin;
      y1Bin = reg3thetaBin + 1;
      if (y0Bin == m_numOfReg3ThetaBins - 1) y1Bin = m_numOfReg3ThetaBins - 1;
    } else if (y >= m_hReg3Theta - 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins))) {
      y0 = m_hReg3Theta - 0.5 * ((m_hReg3Theta - m_lReg3Theta) / (1.0 * m_numOfReg3ThetaBins));
      y1 = m_hReg3Theta;
      y0Bin = m_numOfReg3ThetaBins - 1;
      y1Bin = m_numOfReg3ThetaBins - 1;
    } else {
      y1 = m_hReg3Theta;
      y0Bin = m_numOfReg3ThetaBins - 1;
      y1Bin = m_numOfReg3ThetaBins - 1;
    }
  } //End of region 3
  else {
    B2DEBUG(175, "No valid theta region found (theta=" << theta << " deg), return correction factor of 1.0.");
    return 1.0; // We have not found a theta region that is valid, return no correction at all (i.e. 1.0)
  }

  B2DEBUG(175,  "y0Bin=" << y0Bin << " y1Bin=" << y1Bin);

  //int energyBin = 0;
  double z = energy;
  double z0 = 0.0;
  double z1 = 0.0;
  double zd = 0.0;
  int z0Bin = 0;
  int z1Bin = 1;

  if (z >= 0.00 and z < m_avgRecEn[0]) {
    z0 = 0.01;
    z1 = m_avgRecEn[0];
    z0Bin = 0;
    z1Bin = 0;
  } else if (z >= m_avgRecEn[0] and z < m_avgRecEn[m_numOfEnergyBins - 1]) {
    for (int ii = 0; ii < (m_numOfEnergyBins - 1); ii++) {
      if (z >= m_avgRecEn[ii] and z < m_avgRecEn[ii + 1]) {
        z0 = m_avgRecEn[ii];
        z1 = m_avgRecEn[ii + 1];
        z0Bin = ii;
        z1Bin = ii + 1;
      }
    }
  } else if (z >= m_avgRecEn[m_numOfEnergyBins - 1] and z < 9.0) {
    z0 = m_avgRecEn[m_numOfEnergyBins - 1];
    z1 = 9.0;
    z0Bin = (m_numOfEnergyBins - 1);
    z1Bin = (m_numOfEnergyBins - 1);
  } else {
    z0 = m_avgRecEn[m_numOfEnergyBins - 1];
    z1 = 9.0;
    z0Bin = (m_numOfEnergyBins - 1);
    z1Bin = (m_numOfEnergyBins - 1);
  }

  B2DEBUG(175,  "z0Bin=" << z0Bin << " z1Bin=" << z1Bin);

  xd = (x - x0) / (x1 - x0);
  yd = (y - y0) / (y1 - y0);
  zd = (z - z0) / (z1 - z0);

  if (theta >= m_lReg2Theta && theta <= m_hReg2Theta) {
    double c00, c01, c10, c11, c0, c1;
    c00 = m_reg2CorrFactorArrays[bf][z0Bin][x0Bin][y0Bin] * (1 - xd) + m_reg2CorrFactorArrays[bf][z0Bin][x1Bin][y0Bin] * xd;
    c01 = m_reg2CorrFactorArrays[bf][z1Bin][x0Bin][y0Bin] * (1 - xd) + m_reg2CorrFactorArrays[bf][z1Bin][x1Bin][y0Bin] * xd;
    c10 = m_reg2CorrFactorArrays[bf][z0Bin][x0Bin][y1Bin] * (1 - xd) + m_reg2CorrFactorArrays[bf][z0Bin][x1Bin][y1Bin] * xd;
    c11 = m_reg2CorrFactorArrays[bf][z1Bin][x0Bin][y1Bin] * (1 - xd) + m_reg2CorrFactorArrays[bf][z1Bin][x1Bin][y1Bin] * xd;

    c0 = c00 * (1 - yd) + c10 * yd;
    c1 = c01 * (1 - yd) + c11 * yd;

    result = c0 * (1 - zd) + c1 * zd;
  } else if (theta >= m_lReg1Theta && theta <= m_hReg1Theta) {
    double c00, c01, c10, c11, c0, c1;
    c00 = m_reg1CorrFactorArrays[bf][z0Bin][x0Bin][y0Bin] * (1 - xd) + m_reg1CorrFactorArrays[bf][z0Bin][x1Bin][y0Bin] * xd;
    c01 = m_reg1CorrFactorArrays[bf][z1Bin][x0Bin][y0Bin] * (1 - xd) + m_reg1CorrFactorArrays[bf][z1Bin][x1Bin][y0Bin] * xd;
    c10 = m_reg1CorrFactorArrays[bf][z0Bin][x0Bin][y1Bin] * (1 - xd) + m_reg1CorrFactorArrays[bf][z0Bin][x1Bin][y1Bin] * xd;
    c11 = m_reg1CorrFactorArrays[bf][z1Bin][x0Bin][y1Bin] * (1 - xd) + m_reg1CorrFactorArrays[bf][z1Bin][x1Bin][y1Bin] * xd;

    c0 = c00 * (1 - yd) + c10 * yd;
    c1 = c01 * (1 - yd) + c11 * yd;

    result = c0 * (1 - zd) + c1 * zd;
  } else if (theta >= m_lReg3Theta && theta <= m_hReg3Theta) {
    float c00, c01, c10, c11, c0, c1;
    c00 = m_reg3CorrFactorArrays[bf][z0Bin][x0Bin][y0Bin] * (1 - xd) + m_reg3CorrFactorArrays[bf][z0Bin][x1Bin][y0Bin] * xd;
    c01 = m_reg3CorrFactorArrays[bf][z1Bin][x0Bin][y0Bin] * (1 - xd) + m_reg3CorrFactorArrays[bf][z1Bin][x1Bin][y0Bin] * xd;
    c10 = m_reg3CorrFactorArrays[bf][z0Bin][x0Bin][y1Bin] * (1 - xd) + m_reg3CorrFactorArrays[bf][z0Bin][x1Bin][y1Bin] * xd;
    c11 = m_reg3CorrFactorArrays[bf][z1Bin][x0Bin][y1Bin] * (1 - xd) + m_reg3CorrFactorArrays[bf][z1Bin][x1Bin][y1Bin] * xd;

    c0 = c00 * (1 - yd) + c10 * yd;
    c1 = c01 * (1 - yd) + c11 * yd;

    result = c0 * (1 - zd) + c1 * zd;
  } else {
    result = (m_reg2CorrFactorArrays[bf][z0Bin][0][0] + m_reg2CorrFactorArrays[bf][z1Bin][0][0]) / 2.0;
  }

  return result;
}

