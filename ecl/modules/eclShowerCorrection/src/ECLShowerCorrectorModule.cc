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
#include <framework/logging/Logger.h>

// ROOT
#include <TMath.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dbobjects/ECLShowerCorrectorLeakageCorrection.h>
#include <ecl/dbobjects/ECLShowerEnergyCorrectionTemporary.h>

// MDST
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCorrector)
REG_MODULE(ECLShowerCorrectorPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerCorrectorModule::ECLShowerCorrectorModule() : Module(),
  m_leakageCorrectionPtr_bgx0("ecl_shower_corrector_leakage_corrections"),
  m_leakageCorrectionPtr_thetaGeo_phase2bgx1("ECLLeakageCorrection_thetaGeometry_phase2"),
  m_leakageCorrectionPtr_thetaGeo_phase3bgx1("ECLLeakageCorrection_thetaGeometry_phase3"),
  m_leakageCorrectionPtr_phiGeo_phase2bgx1("ECLLeakageCorrection_phiGeometry_phase2"),
  m_leakageCorrectionPtr_phiGeo_phase3bgx1("ECLLeakageCorrection_phiGeometry_phase3"),
  m_leakageCorrectionPtr_thetaEn_phase2bgx1("ECLLeakageCorrection_thetaEnergy_phase2"),
  m_leakageCorrectionPtr_thetaEn_phase3bgx1("ECLLeakageCorrection_thetaEnergy_phase3"),
  m_leakageCorrectionPtr_phiEn_phase2bgx1("ECLLeakageCorrection_phiEnergy_phase2"),
  m_leakageCorrectionPtr_phiEn_phase3bgx1("ECLLeakageCorrection_phiEnergy_phase3"),
  m_eclShowers(eclShowerArrayName()),
  m_eventLevelClusteringInfo(eventLevelClusteringInfoName())
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
  m_eventLevelClusteringInfo.registerInDataStore(eventLevelClusteringInfoName());
}

void ECLShowerCorrectorModule::beginRun()
{
  // TODO: callback!
  prepareLeakageCorrections();
}

void ECLShowerCorrectorModule::event()
{

  // Get the event background level.
  const int bkgdcount = m_eventLevelClusteringInfo->getNECLCalDigitsOutOfTime();
  double backgroundLevel = 0.0; // from out of time digit counting
  if (m_fullBkgdCount > 0) {
    backgroundLevel = static_cast<double>(bkgdcount) / m_fullBkgdCount;
  }

  // Loop over all ECLShowers.
  for (auto& eclShower : m_eclShowers) {

    // Only correct EM showers! Other showers keep the raw energy!
    if (eclShower.getHypothesisId() == ECLShower::c_nPhotons) {

      const double energy        = eclShower.getEnergy();
      const double energyHighest = eclShower.getEnergyHighestCrystal();
      const double theta         = eclShower.getTheta() * TMath::RadToDeg();
      const double phi           = eclShower.getPhi() * TMath::RadToDeg();

      // Get the correction
      double correctionFactor = 1.0;

      if (backgroundLevel < 0.1) correctionFactor = getLeakageCorrection(theta, phi, energy, backgroundLevel); // "Sumans corrections"
      else correctionFactor = getLeakageCorrectionTemporary(theta, phi, energy, backgroundLevel); // "Elisas and Claudias corrections"

      B2DEBUG(175, "theta=" << theta << ", phi=" << phi << ", E=" << energy << ", BG=" << backgroundLevel << " --> correction factor=" <<
              correctionFactor);

      if (correctionFactor < 1.e-5 or correctionFactor > 10.) {
        B2ERROR("Correction factor=" << correctionFactor << " is very small/too large! Resetting to 1.0.");
        correctionFactor = 1.0;
      }

      const double correctedEnergy = energy * correctionFactor;
      const double correctedEnergyHighest = energyHighest * correctionFactor;
      B2DEBUG(175, "Correction factor=" << correctionFactor << ", correctedEnergy=" << correctedEnergy << ", correctedEnergyHighest=" <<
              correctedEnergyHighest);

      // Set the correction
      eclShower.setEnergy(correctedEnergy);
      eclShower.setEnergyHighestCrystal(correctedEnergyHighest);

    } // end correction
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
  //thetaGeometry phase2
  m_leakage_bgx1[0] = m_leakageCorrectionPtr_thetaGeo_phase2bgx1->getGraph2D();
  m_leakage_bgx1_limits[0].resize(4);
  m_leakage_bgx1_limits[0][0] = m_leakageCorrectionPtr_thetaGeo_phase2bgx1->getAngleMin();
  m_leakage_bgx1_limits[0][1] = m_leakageCorrectionPtr_thetaGeo_phase2bgx1->getAngleMax();
  m_leakage_bgx1_limits[0][2] = m_leakageCorrectionPtr_thetaGeo_phase2bgx1->getEnergyMin();
  m_leakage_bgx1_limits[0][3] = m_leakageCorrectionPtr_thetaGeo_phase2bgx1->getEnergyMax();

  //thetaGeometry phase3
  m_leakage_bgx1[1] = m_leakageCorrectionPtr_thetaGeo_phase3bgx1->getGraph2D();
  m_leakage_bgx1_limits[1].resize(4);
  m_leakage_bgx1_limits[1][0] = m_leakageCorrectionPtr_thetaGeo_phase3bgx1->getAngleMin();
  m_leakage_bgx1_limits[1][1] = m_leakageCorrectionPtr_thetaGeo_phase3bgx1->getAngleMax();
  m_leakage_bgx1_limits[1][2] = m_leakageCorrectionPtr_thetaGeo_phase3bgx1->getEnergyMin();
  m_leakage_bgx1_limits[1][3] = m_leakageCorrectionPtr_thetaGeo_phase3bgx1->getEnergyMax();
  B2DEBUG(175, "Prepariong leakage corrections, theta and energy  boundaries :  thetamin=" << m_leakage_bgx1_limits[0][0] <<
          ", thate_max=" << m_leakage_bgx1_limits[0][1]
          << ", Emin=" << m_leakage_bgx1_limits[0][2] << " Emax= " << m_leakage_bgx1_limits[0][3]);

  //phiGeometry phase2
  m_leakage_bgx1[2] = m_leakageCorrectionPtr_phiGeo_phase2bgx1->getGraph2D();
  m_leakage_bgx1_limits[2].resize(4);
  m_leakage_bgx1_limits[2][0] = m_leakageCorrectionPtr_phiGeo_phase2bgx1->getAngleMin();
  m_leakage_bgx1_limits[2][1] = m_leakageCorrectionPtr_phiGeo_phase2bgx1->getAngleMax();
  m_leakage_bgx1_limits[2][2] = m_leakageCorrectionPtr_phiGeo_phase2bgx1->getEnergyMin();
  m_leakage_bgx1_limits[2][3] = m_leakageCorrectionPtr_phiGeo_phase2bgx1->getEnergyMax();

  //phiGeometry phase3
  m_leakage_bgx1[3] = m_leakageCorrectionPtr_phiGeo_phase3bgx1->getGraph2D();
  m_leakage_bgx1_limits[3].resize(4);
  m_leakage_bgx1_limits[3][0] = m_leakageCorrectionPtr_phiGeo_phase3bgx1->getAngleMin();
  m_leakage_bgx1_limits[3][1] = m_leakageCorrectionPtr_phiGeo_phase3bgx1->getAngleMax();
  m_leakage_bgx1_limits[3][2] = m_leakageCorrectionPtr_phiGeo_phase3bgx1->getEnergyMin();
  m_leakage_bgx1_limits[3][3] = m_leakageCorrectionPtr_phiGeo_phase3bgx1->getEnergyMax();

  //thetaEnergy phase2
  m_leakage_bgx1[4] = m_leakageCorrectionPtr_thetaEn_phase2bgx1->getGraph2D();
  m_leakage_bgx1_limits[4].resize(4);
  m_leakage_bgx1_limits[4][0] = m_leakageCorrectionPtr_thetaEn_phase2bgx1->getAngleMin();
  m_leakage_bgx1_limits[4][1] = m_leakageCorrectionPtr_thetaEn_phase2bgx1->getAngleMax();
  m_leakage_bgx1_limits[4][2] = m_leakageCorrectionPtr_thetaEn_phase2bgx1->getEnergyMin();
  m_leakage_bgx1_limits[4][3] = m_leakageCorrectionPtr_thetaEn_phase2bgx1->getEnergyMax();

  //thetaEnergy phase3
  m_leakage_bgx1[5] = m_leakageCorrectionPtr_thetaEn_phase3bgx1->getGraph2D();
  m_leakage_bgx1_limits[5].resize(4);
  m_leakage_bgx1_limits[5][0] = m_leakageCorrectionPtr_thetaEn_phase3bgx1->getAngleMin();
  m_leakage_bgx1_limits[5][1] = m_leakageCorrectionPtr_thetaEn_phase3bgx1->getAngleMax();
  m_leakage_bgx1_limits[5][2] = m_leakageCorrectionPtr_thetaEn_phase3bgx1->getEnergyMin();
  m_leakage_bgx1_limits[5][3] = m_leakageCorrectionPtr_thetaEn_phase3bgx1->getEnergyMax();

  //phiEnergy phase2
  m_leakage_bgx1[6] = m_leakageCorrectionPtr_phiEn_phase2bgx1->getGraph2D();
  m_leakage_bgx1_limits[6].resize(4);
  m_leakage_bgx1_limits[6][0] = m_leakageCorrectionPtr_phiEn_phase2bgx1->getAngleMin();
  m_leakage_bgx1_limits[6][1] = m_leakageCorrectionPtr_phiEn_phase2bgx1->getAngleMax();
  m_leakage_bgx1_limits[6][2] = m_leakageCorrectionPtr_phiEn_phase2bgx1->getEnergyMin();
  m_leakage_bgx1_limits[6][3] = m_leakageCorrectionPtr_phiEn_phase2bgx1->getEnergyMax();

  //phiEnergy phase3
  m_leakage_bgx1[7] = m_leakageCorrectionPtr_phiEn_phase3bgx1->getGraph2D();
  m_leakage_bgx1_limits[7].resize(4);
  m_leakage_bgx1_limits[7][0] = m_leakageCorrectionPtr_phiEn_phase3bgx1->getAngleMin();
  m_leakage_bgx1_limits[7][1] = m_leakageCorrectionPtr_phiEn_phase3bgx1->getAngleMax();
  m_leakage_bgx1_limits[7][2] = m_leakageCorrectionPtr_phiEn_phase3bgx1->getEnergyMin();
  m_leakage_bgx1_limits[7][3] = m_leakageCorrectionPtr_phiEn_phase3bgx1->getEnergyMax();

  // Prepare energy correction constants taken from the database to be used in an interpolation correction
  // get all information from the payload
  m_numOfBfBins        = m_leakageCorrectionPtr_bgx0->getNumOfBfBins()[0];
  m_numOfEnergyBins    = m_leakageCorrectionPtr_bgx0->getNumOfEnergyBins()[0];
  m_numOfPhiBins       = m_leakageCorrectionPtr_bgx0->getNumOfPhiBins()[0];
  m_numOfReg1ThetaBins = m_leakageCorrectionPtr_bgx0->getNumOfReg1ThetaBins()[0];
  m_numOfReg2ThetaBins = m_leakageCorrectionPtr_bgx0->getNumOfReg2ThetaBins()[0];
  m_numOfReg3ThetaBins = m_leakageCorrectionPtr_bgx0->getNumOfReg3ThetaBins()[0];

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

  m_avgRecEn = m_leakageCorrectionPtr_bgx0->getAvgRecEn();

  // Fill the multidimensional vectors
  m_correctionFactor = m_leakageCorrectionPtr_bgx0->getCorrectionFactor();
  m_bgFractionBinNum = m_leakageCorrectionPtr_bgx0->getBgFractionBinNum();
  m_regNum = m_leakageCorrectionPtr_bgx0->getRegNum();
  m_phiBinNum = m_leakageCorrectionPtr_bgx0->getPhiBinNum();
  m_thetaBinNum = m_leakageCorrectionPtr_bgx0->getThetaBinNum();
  m_energyBinNum = m_leakageCorrectionPtr_bgx0->getEnergyBinNum();
  m_correctionFactor = m_leakageCorrectionPtr_bgx0->getCorrectionFactor();

  for (unsigned i = 0; i < m_correctionFactor.size(); ++i) {
    if (m_regNum[i] == 1) {
      m_reg1CorrFactorArrays[m_bgFractionBinNum[i]][m_energyBinNum[i]][m_phiBinNum[i]][m_thetaBinNum[i]] = m_correctionFactor[i];
    } else if (m_regNum[i] == 2) {
      m_reg2CorrFactorArrays[m_bgFractionBinNum[i]][m_energyBinNum[i]][m_phiBinNum[i]][m_thetaBinNum[i]] = m_correctionFactor[i];
    } else if (m_regNum[i] == 3) {
      m_reg3CorrFactorArrays[m_bgFractionBinNum[i]][m_energyBinNum[i]][m_phiBinNum[i]][m_thetaBinNum[i]] = m_correctionFactor[i];
    }
  }

  m_phiPeriodicity = m_leakageCorrectionPtr_bgx0->getPhiPeriodicity()[0];
  m_lReg1Theta = m_leakageCorrectionPtr_bgx0->getLReg1Theta()[0];
  m_hReg1Theta = m_leakageCorrectionPtr_bgx0->getHReg1Theta()[0];
  m_lReg2Theta = m_leakageCorrectionPtr_bgx0->getLReg2Theta()[0];
  m_hReg2Theta = m_leakageCorrectionPtr_bgx0->getHReg2Theta()[0];
  m_lReg3Theta = m_leakageCorrectionPtr_bgx0->getLReg3Theta()[0];
  m_hReg3Theta = m_leakageCorrectionPtr_bgx0->getHReg3Theta()[0];

}

double ECLShowerCorrectorModule::getLeakageCorrectionTemporary(const double theta,
    const double phi,
    const double energy,
    const double background)
{
  // Corrections are available for Phase2BG15x1 and Phase3BG15x1.
  int add_type = 0;
  if (background > 0.75) add_type = 1;

  // m_leakage_bgx1_limits are ordered this way:
  // 0 : theta_geo ph2, 1 : theta_geo ph3, 2 : phi_geo ph2,   3 : phi_geo ph3,
  // 4 : theta_en ph2, 5 : theta_en ph3, 6 : phi_en ph2,   7 : phi_en ph3
  int type_theta_geo = 0 + add_type;
  int type_phi_geo = 2 + add_type;
  int type_theta_en = 4 + add_type ;
  int type_phi_en = 6 + add_type;

  B2DEBUG(175, "Index to select payloads: type_theta_geo = " << type_theta_geo << " , type_phi_geo =  " << type_phi_geo
          << " type_theta_en = " << type_theta_en << " , type_phi_en = " << type_phi_en);

  double theta_clip = theta;
  double phi_clip = phi;
  double energy_clip = energy;

  B2DEBUG(175, "Cluster info to compute leakage corrections: theta=" << theta_clip << ", phi=" << phi_clip << ", E=" << energy_clip);

  // check and clip boundaries since TGraph2D returns zero outside of them
  if (theta_clip < m_leakage_bgx1_limits[type_theta_geo][0]) theta_clip = m_leakage_bgx1_limits[type_theta_geo][0] + 1e-5;
  if (theta_clip > m_leakage_bgx1_limits[type_theta_geo][1]) theta_clip = m_leakage_bgx1_limits[type_theta_geo][1] - 1e-5;
  if (energy_clip < m_leakage_bgx1_limits[type_theta_geo][2]) energy_clip = m_leakage_bgx1_limits[type_theta_geo][2] + 1e-5;
  if (energy_clip > m_leakage_bgx1_limits[type_theta_geo][3]) energy_clip = m_leakage_bgx1_limits[type_theta_geo][3] - 1e-5;
  double corr_theta_geo = m_leakage_bgx1[type_theta_geo].Interpolate(theta_clip, energy_clip);
  theta_clip = theta; energy_clip = energy; // re-initialize for the computetion of the remaining corrections

  if (phi_clip < m_leakage_bgx1_limits[type_phi_geo][0]) phi_clip = m_leakage_bgx1_limits[type_phi_geo][0] + 1e-5;
  if (phi_clip > m_leakage_bgx1_limits[type_phi_geo][1]) phi_clip = m_leakage_bgx1_limits[type_phi_geo][1] - 1e-5;
  if (energy_clip < m_leakage_bgx1_limits[type_phi_geo][2]) energy_clip = m_leakage_bgx1_limits[type_phi_geo][2] + 1e-5;
  if (energy_clip > m_leakage_bgx1_limits[type_phi_geo][3]) energy_clip = m_leakage_bgx1_limits[type_phi_geo][3] - 1e-5;
  double corr_phi_geo = m_leakage_bgx1[type_phi_geo].Interpolate(phi_clip, energy_clip);
  phi_clip = phi; energy_clip = energy;

  if (theta_clip < m_leakage_bgx1_limits[type_theta_en][0]) theta_clip = m_leakage_bgx1_limits[type_theta_en][0] + 1e-5;
  if (theta_clip > m_leakage_bgx1_limits[type_theta_en][1]) theta_clip = m_leakage_bgx1_limits[type_theta_en][1] - 1e-5;
  if (energy_clip < m_leakage_bgx1_limits[type_theta_en][2]) energy_clip = m_leakage_bgx1_limits[type_theta_en][2] + 1e-5;
  if (energy_clip > m_leakage_bgx1_limits[type_theta_en][3]) energy_clip = m_leakage_bgx1_limits[type_theta_en][3] - 1e-5;
  double corr_theta_en = m_leakage_bgx1[type_theta_en].Interpolate(theta_clip, energy_clip);
  theta_clip = theta; energy_clip = energy;

  if (phi_clip < m_leakage_bgx1_limits[type_phi_en][0]) phi_clip = m_leakage_bgx1_limits[type_phi_en][0] + 1e-5;
  if (phi_clip > m_leakage_bgx1_limits[type_phi_en][1]) phi_clip = m_leakage_bgx1_limits[type_phi_en][1] - 1e-5;
  if (energy_clip < m_leakage_bgx1_limits[type_phi_en][2]) energy_clip = m_leakage_bgx1_limits[type_phi_en][2] + 1e-5;
  if (energy_clip > m_leakage_bgx1_limits[type_phi_en][3]) energy_clip = m_leakage_bgx1_limits[type_phi_en][3] - 1e-5;
  double corr_phi_en = m_leakage_bgx1[type_phi_en].Interpolate(phi_clip, energy_clip);


  // The final correction formula is:
  // correctedEnergy = (rawEnergy + corr_theta_geo+corr_phi_geo ) *corr_theta_en*corr_phi_en
  // to use "corr" as multiplicative factor in ECLShowerCorrectorModule::event,
  // the following form should be returned
  double corr = (1 + (corr_theta_geo + corr_phi_geo) / energy_clip) * corr_theta_en * corr_phi_en;

  B2DEBUG(175, "Geometrical correction factors: corr_theta_geo=" << corr_theta_geo << ", corr_phi_geo=" << corr_phi_geo);
  B2DEBUG(175, "Energy correction factors:  corr_theta_en=" << corr_theta_en << ", corr_phi_en=" << corr_phi_en);
  B2DEBUG(175, "Final correction=" << corr);

  return corr;

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
  double x0 = 0.0;
  double x1 = 0.0;
  double xd = 0.0;

  // Convert -180..180 to 0..360
  const double phiMod = phi + 180;

  int x0Bin = 0;
  int x1Bin = m_numOfPhiBins - 1;
  int phiGap = int(phiMod / (360.0 / (m_phiPeriodicity * 1.0)));
  double x = phiMod - phiGap * (360.0 / (m_phiPeriodicity * 1.0));

  const double phiBinWidth = 360.0 / (1.0 * m_numOfPhiBins * m_phiPeriodicity);

  if (x <= phiBinWidth / 2.0) {
    x0 = 0.0;
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

  if (z >= 0.0 and z < m_avgRecEn[0]) {
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
