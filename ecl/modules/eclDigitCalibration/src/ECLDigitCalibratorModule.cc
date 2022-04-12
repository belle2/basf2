/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclDigitCalibration/ECLDigitCalibratorModule.h>

//STL
#include <unordered_map>

// ROOT
#include "TH1F.h"
#include "TFile.h"

// ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/dataobjects/ECLPureCsIInfo.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/utility/utilityFunctions.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

// FRAMEWORK
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/core/Environment.h>

//MDST
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLDigitCalibrator)
REG_MODULE(ECLDigitCalibratorPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
ECLDigitCalibratorModule::ECLDigitCalibratorModule() :
  m_calibrationCrystalElectronics("ECLCrystalElectronics"),
  m_calibrationCrystalEnergy("ECLCrystalEnergy"),
  m_calibrationCrystalElectronicsTime("ECLCrystalElectronicsTime"),
  m_calibrationCrystalTimeOffset("ECLCrystalTimeOffset"),
  m_calibrationCrateTimeOffset("ECLCrateTimeOffset"),
  m_calibrationCrystalFlightTime("ECLCrystalFlightTime"),
  m_eclDigits(eclDigitArrayName()),
  m_eclCalDigits(eclCalDigitArrayName()),
  m_eclDsps(eclDspArrayName()),
  m_eclPureCsIInfo(eclPureCsIInfoArrayName())
{
  // Set module properties
  setDescription("Applies digit energy, time and time-resolution calibration to each ECL digit. Counts number of out-of-time background digits to determine the event-by-event background level.");
  addParam("backgroundEnergyCut", m_backgroundEnergyCut, "Energy cut used to count background digits", 7.0 * Belle2::Unit::MeV);
  addParam("backgroundTimingCut", m_backgroundTimingCut, "Timing cut used to count background digits", 110.0 * Belle2::Unit::ns);
  addParam("fileBackgroundName", m_fileBackgroundName, "Background filename.",
           FileSystem::findFile("/data/ecl/background_norm.root"));
  addParam("simulatePure", m_simulatePure, "Flag to simulate pure CsI option", false);

  // t-t0 = p1 + pow( (p3/(amplitude+p2)), p4 ) + p5*exp(-amplitude/p6)      ("Energy dependence equation")
  addParam("energyDependenceTimeOffsetFitParam_p1", m_energyDependenceTimeOffsetFitParam_p1,
           "Fit parameter (p1) for applying correction to the time offset as a function of the energy (amplitude)", -999.0);
  addParam("energyDependenceTimeOffsetFitParam_p2", m_energyDependenceTimeOffsetFitParam_p2,
           "Fit parameter (p2) for applying correction to the time offset as a function of the energy (amplitude)", -999.0);
  addParam("energyDependenceTimeOffsetFitParam_p3", m_energyDependenceTimeOffsetFitParam_p3,
           "Fit parameter (p3) for applying correction to the time offset as a function of the energy (amplitude)", -999.0);
  addParam("energyDependenceTimeOffsetFitParam_p4", m_energyDependenceTimeOffsetFitParam_p4,
           "Fit parameter (p4) for applying correction to the time offset as a function of the energy (amplitude)", -999.0);
  addParam("energyDependenceTimeOffsetFitParam_p5", m_energyDependenceTimeOffsetFitParam_p5,
           "Fit parameter (p5) for applying correction to the time offset as a function of the energy (amplitude)", -999.0);
  addParam("energyDependenceTimeOffsetFitParam_p6", m_energyDependenceTimeOffsetFitParam_p6,
           "Fit parameter (p6) for applying correction to the time offset as a function of the energy (amplitude)", -999.0);


  // Parallel processing certification
  setPropertyFlags(c_ParallelProcessingCertified);

  m_averageBG = 0;
  m_pol2Max = 0.0;
  m_timeInverseSlope = 0.0;

}

// destructor
ECLDigitCalibratorModule::~ECLDigitCalibratorModule()
{
}

// initialize calibration
void ECLDigitCalibratorModule::initializeCalibration()
{

  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency. Same for all crystals:
  m_timeInverseSlope = 1.0 / (4.0 * EclConfiguration::getRF()) * 1e3;
  m_pureCsIEnergyCalib = 0.00005; //conversion factor from ADC counts to GeV
  m_pureCsITimeCalib = 0.1; //conversion factor from eclPureCsIDigitizer to ns
  m_pureCsITimeOffset = 0.31; //ad-hoc offset correction for pureCsI timing

  callbackCalibration(m_calibrationCrystalElectronics, v_calibrationCrystalElectronics, v_calibrationCrystalElectronicsUnc);
  callbackCalibration(m_calibrationCrystalEnergy, v_calibrationCrystalEnergy, v_calibrationCrystalEnergyUnc);
  callbackCalibration(m_calibrationCrystalElectronicsTime, v_calibrationCrystalElectronicsTime,
                      v_calibrationCrystalElectronicsTimeUnc);
  callbackCalibration(m_calibrationCrystalTimeOffset, v_calibrationCrystalTimeOffset, v_calibrationCrystalTimeOffsetUnc);
  callbackCalibration(m_calibrationCrateTimeOffset, v_calibrationCrateTimeOffset, v_calibrationCrateTimeOffsetUnc);
  callbackCalibration(m_calibrationCrystalFlightTime, v_calibrationCrystalFlightTime, v_calibrationCrystalFlightTimeUnc);
}

// callback calibration
void ECLDigitCalibratorModule::callbackCalibration(DBObjPtr<ECLCrystalCalib>& cal, std::vector<float>& constants,
                                                   std::vector<float>& constantsUnc)
{
  constants = cal->getCalibVector();
  constantsUnc = cal->getCalibUncVector();
}


// initialize
void ECLDigitCalibratorModule::initialize()
{
  //mdst dataobjects
  m_eventLevelClusteringInfo.registerInDataStore(eventLevelClusteringInfoName());

  // Register Digits, CalDigits and their relation in datastore
  m_eclDigits.registerInDataStore(eclDigitArrayName());
  m_eclDsps.registerInDataStore(eclDspArrayName());
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  m_eclCalDigits.registerRelationTo(m_eclDigits);

  //Special information for pure CsI simulation
  m_eclPureCsIInfo.registerInDataStore(eclPureCsIInfoArrayName());

  // initialize calibration
  initializeCalibration();

  // initialize time resolution (not yet from the database)
  // read the Background correction factors (for full background)
  m_fileBackground = new TFile(m_fileBackgroundName.c_str(), "READ");
  if (!m_fileBackground) B2FATAL("Could not find file: " << m_fileBackgroundName);
  m_th1fBackground = dynamic_cast<TH1F*>(m_fileBackground->Get("background"));
  if (!m_th1fBackground) B2FATAL("Could not find m_th1fBackground");

  // average BG value from m_th1fBackground
  m_averageBG = m_th1fBackground->Integral() / m_th1fBackground->GetEntries();

  // get maximum position ("x") of 2-order pol background for t99
  if (fabs(c_pol2Var3) > 1e-12) {
    m_pol2Max = -c_pol2Var2 / (2 * c_pol2Var3);
  } else {
    m_pol2Max = 0.;
  }

  if ((m_energyDependenceTimeOffsetFitParam_p1 != -999) &&
      (m_energyDependenceTimeOffsetFitParam_p2 != -999) &&
      (m_energyDependenceTimeOffsetFitParam_p3 != -999) &&
      (m_energyDependenceTimeOffsetFitParam_p4 != -999) &&
      (m_energyDependenceTimeOffsetFitParam_p5 != -999) &&
      (m_energyDependenceTimeOffsetFitParam_p6 != -999)) {
    B2DEBUG(80, "m_energyDependenceTimeOffsetFitParam_p1 = " << m_energyDependenceTimeOffsetFitParam_p1);
    B2DEBUG(80, "m_energyDependenceTimeOffsetFitParam_p2 = " << m_energyDependenceTimeOffsetFitParam_p2);
    B2DEBUG(80, "m_energyDependenceTimeOffsetFitParam_p3 = " << m_energyDependenceTimeOffsetFitParam_p3);
    B2DEBUG(80, "m_energyDependenceTimeOffsetFitParam_p4 = " << m_energyDependenceTimeOffsetFitParam_p4);
    B2DEBUG(80, "m_energyDependenceTimeOffsetFitParam_p5 = " << m_energyDependenceTimeOffsetFitParam_p5);
    B2DEBUG(80, "m_energyDependenceTimeOffsetFitParam_p6 = " << m_energyDependenceTimeOffsetFitParam_p6);

    ECLTimeUtil->setTimeWalkFuncParams(m_energyDependenceTimeOffsetFitParam_p1,
                                       m_energyDependenceTimeOffsetFitParam_p2,
                                       m_energyDependenceTimeOffsetFitParam_p3,
                                       m_energyDependenceTimeOffsetFitParam_p4,
                                       m_energyDependenceTimeOffsetFitParam_p5,
                                       m_energyDependenceTimeOffsetFitParam_p6) ;
  }
}

// begin run
void ECLDigitCalibratorModule::beginRun()
{
  // Check if any of the calibration constants have changed
  if (m_calibrationCrystalElectronics.hasChanged()) {
    if (m_calibrationCrystalElectronics) {
      callbackCalibration(m_calibrationCrystalElectronics, v_calibrationCrystalElectronics, v_calibrationCrystalElectronicsUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrystalElectronics for current run!");
  }

  if (m_calibrationCrystalEnergy.hasChanged()) {
    if (m_calibrationCrystalEnergy) {
      callbackCalibration(m_calibrationCrystalEnergy, v_calibrationCrystalEnergy, v_calibrationCrystalEnergyUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrystalEnergy for current run!");
  }

  if (m_calibrationCrystalElectronicsTime.hasChanged()) {
    if (m_calibrationCrystalElectronicsTime) {
      callbackCalibration(m_calibrationCrystalElectronicsTime, v_calibrationCrystalElectronicsTime,
                          v_calibrationCrystalElectronicsTimeUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrystalElectronicsTime for current run!");
  }

  if (m_calibrationCrystalTimeOffset.hasChanged()) {
    if (m_calibrationCrystalTimeOffset) {
      callbackCalibration(m_calibrationCrystalTimeOffset, v_calibrationCrystalTimeOffset, v_calibrationCrystalTimeOffsetUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrystalTimeOffset for current run!");
  }

  if (m_calibrationCrateTimeOffset.hasChanged()) {
    if (m_calibrationCrateTimeOffset) {
      callbackCalibration(m_calibrationCrateTimeOffset, v_calibrationCrateTimeOffset, v_calibrationCrateTimeOffsetUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrateTimeOffset for current run!");
  }

  if (m_calibrationCrystalFlightTime.hasChanged()) {
    if (m_calibrationCrystalFlightTime) {
      callbackCalibration(m_calibrationCrystalFlightTime, v_calibrationCrystalFlightTime, v_calibrationCrystalFlightTimeUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrystalFlightTime for current run!");
  }

}

// event
void ECLDigitCalibratorModule::event()
{

  // Loop over the input array
  for (auto& aECLDigit : m_eclDigits) {

    bool is_pure_csi = 0;

    // append an ECLCalDigit to the storearray
    const auto aECLCalDigit = m_eclCalDigits.appendNew();

    // get the cell id from the ECLDigit as identifier
    const int cellid = aECLDigit.getCellId();

    // check that the cell id is valid
    if (cellid < 1 or cellid > c_nCrystals) {
      B2FATAL("ECLDigitCalibrationModule::event():" << cellid << " out of range!");
    }

    // perform the digit energy calibration: E = A * Ce * Cs
    const int amplitude = aECLDigit.getAmp();
    double calibratedEnergy = 0;

    if (m_simulatePure) {
      if (aECLDigit.getRelated<ECLPureCsIInfo>(eclPureCsIInfoArrayName()) != nullptr) {
        if (aECLDigit.getRelated<ECLPureCsIInfo>(eclPureCsIInfoArrayName())->getPureCsI())
          is_pure_csi = 1;
      }
    }

    if (is_pure_csi) {
      calibratedEnergy = amplitude * m_pureCsIEnergyCalib;
    } else {
      calibratedEnergy = amplitude * v_calibrationCrystalElectronics[cellid - 1] * v_calibrationCrystalEnergy[cellid - 1];
    }
    if (calibratedEnergy < 0.0) calibratedEnergy = 0.0;

    // perform the digit timing calibration: t = c * (tfit - Te - Ts)
    const int time = aECLDigit.getTimeFit();
    const int quality = aECLDigit.getQuality();
    double calibratedTime = c_timeForFitFailed;
    if (quality == 1) {
      aECLCalDigit->addStatus(ECLCalDigit::c_IsFailedFit); //this is used to flag failed fits
    } else { //only calibrate digit time if we have a good waveform fit
      if (is_pure_csi) {
        calibratedTime = m_pureCsITimeCalib * m_timeInverseSlope * (time - v_calibrationCrystalElectronicsTime[cellid - 1] -
                                                                    v_calibrationCrystalTimeOffset[cellid - 1] -
                                                                    v_calibrationCrateTimeOffset[cellid - 1])
                         - v_calibrationCrystalFlightTime[cellid - 1] + m_pureCsITimeOffset ;
      } else {
        calibratedTime = m_timeInverseSlope * (time - v_calibrationCrystalElectronicsTime[cellid - 1] -
                                               v_calibrationCrystalTimeOffset[cellid - 1] -
                                               v_calibrationCrateTimeOffset[cellid - 1])
                         - v_calibrationCrystalFlightTime[cellid - 1] ;
      }

      // For data, apply a correction to the time as a function of the signal amplitude.  Correction determined from a fit.
      // No correction for MC
      bool m_IsMCFlag = Environment::Instance().isMC();
      B2DEBUG(35, "cellid = " << cellid << ", m_IsMCFlag = " << m_IsMCFlag) ;

      if (!m_IsMCFlag) {
        double energyTimeShift = ECLTimeUtil->energyDependentTimeOffsetElectronic(amplitude * v_calibrationCrystalElectronics[cellid - 1]) *
                                 m_timeInverseSlope ;
        B2DEBUG(35, "cellid = " << cellid << ", amplitude = " << amplitude << ", corrected amplitude = " << amplitude *
                v_calibrationCrystalElectronics[cellid - 1] << ", time before t(E) shift = " << calibratedTime << ", t(E) shift = " <<
                energyTimeShift << " ns") ;
        calibratedTime -= energyTimeShift ;
      }
    }

    B2DEBUG(35, "cellid = " << cellid << ", amplitude = " << amplitude << ", calibrated energy = " << calibratedEnergy);
    B2DEBUG(35, "cellid = " << cellid << ", time = " << time << ", calibratedTime = " << calibratedTime);

    //Calibrating offline fit results
    ECLDsp* aECLDsp = ECLDsp::getByCellID(cellid);
    aECLCalDigit->setTwoComponentChi2(-1);
    aECLCalDigit->setTwoComponentSavedChi2(ECLDsp::photonHadron, -1);
    aECLCalDigit->setTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton, -1);
    aECLCalDigit->setTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing, -1);
    aECLCalDigit->setTwoComponentTotalEnergy(-1);
    aECLCalDigit->setTwoComponentHadronEnergy(-1);
    aECLCalDigit->setTwoComponentDiodeEnergy(-1);
    if (aECLDsp) {
      //require ECLDigit to have offline waveform
      if (aECLDsp->getTwoComponentChi2() > 0) {
        //require offline waveform to have offline fit result

        const double calibratedTwoComponentTotalEnergy = aECLDsp->getTwoComponentTotalAmp() * v_calibrationCrystalElectronics[cellid - 1] *
                                                         v_calibrationCrystalEnergy[cellid - 1];
        const double calibratedTwoComponentHadronEnergy = aECLDsp->getTwoComponentHadronAmp() * v_calibrationCrystalElectronics[cellid -
                                                          1] *
                                                          v_calibrationCrystalEnergy[cellid - 1];
        const double calibratedTwoComponentDiodeEnergy = aECLDsp->getTwoComponentDiodeAmp() * v_calibrationCrystalElectronics[cellid - 1] *
                                                         v_calibrationCrystalEnergy[cellid - 1];
        const double twoComponentChi2 = aECLDsp->getTwoComponentChi2();
        const ECLDsp::TwoComponentFitType twoComponentFitType = aECLDsp->getTwoComponentFitType();

        aECLCalDigit->setTwoComponentTotalEnergy(calibratedTwoComponentTotalEnergy);
        aECLCalDigit->setTwoComponentHadronEnergy(calibratedTwoComponentHadronEnergy);
        aECLCalDigit->setTwoComponentDiodeEnergy(calibratedTwoComponentDiodeEnergy);
        aECLCalDigit->setTwoComponentChi2(twoComponentChi2);
        aECLCalDigit->setTwoComponentSavedChi2(ECLDsp::photonHadron, aECLDsp->getTwoComponentSavedChi2(ECLDsp::photonHadron));
        aECLCalDigit->setTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton,
                                               aECLDsp->getTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton));
        aECLCalDigit->setTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing, aECLDsp->getTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing));
        aECLCalDigit->setTwoComponentFitType(twoComponentFitType);

      }
    }

    // fill the ECLCalDigit with the cell id, the calibrated information and calibration status
    aECLCalDigit->setCellId(cellid);

    aECLCalDigit->setEnergy(calibratedEnergy);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsEnergyCalibrated);

    aECLCalDigit->setTime(calibratedTime);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsTimeCalibrated);

    // set a relation to the ECLDigit
    aECLCalDigit->addRelationTo(&aECLDigit);
  }

  // determine background level
  const int bgCount = determineBackgroundECL();

  // set the t99 (time resolution)
  for (auto& aECLCalDigit : m_eclCalDigits) {

    // perform the time resolution calibration
    const double t99 = getT99(aECLCalDigit.getCellId(),
                              aECLCalDigit.getEnergy(),
                              aECLCalDigit.hasStatus(ECLCalDigit::c_IsFailedFit),
                              bgCount); // calibrated time resolution
    aECLCalDigit.setTimeResolution(t99);

    if (t99 == c_timeResolutionForFitFailed) {
      aECLCalDigit.addStatus(ECLCalDigit::c_IsFailedTimeResolution);
    }

    aECLCalDigit.addStatus(ECLCalDigit::c_IsTimeResolutionCalibrated);
  }
}

// end run
void ECLDigitCalibratorModule::endRun()
{

}

// terminate
void ECLDigitCalibratorModule::terminate()
{

}

// Time resolution calibration
double ECLDigitCalibratorModule::getT99(const int cellid, const double energy, const bool fitfailed, const int bgcount) const
{

  // if this digit is calibrated to the trigger time, we set the resolution to 'very bad' (to be discussed)
  if (fitfailed) return c_timeResolutionForFitFailed;

  // Get the background level [MeV / mus]
  const double bglevel = TMath::Min((double) bgcount / (double) c_nominalBG * m_th1fBackground->GetBinContent(cellid) / m_averageBG,
                                    m_pol2Max); // c_nominalBG = 183 for actual version of digitizer, m_averageBG is about 2 MeV/ mus

  // Get p1 as function of background level
  const double p1 = c_pol2Var1 + c_pol2Var2 * bglevel + c_pol2Var3 * bglevel * bglevel;

  // inverse energy in 1/MeV
  double einv = 0.;
  if (energy > 1e-12) einv = 1. / (energy / Belle2::Unit::MeV);

  // calculate t99 using the inverse energy and p1 (p0 is zero)
  double t99 = p1 * einv;

  // for high energies we fix t99 to 3.5ns
  if (t99 < c_minT99) t99 = c_minT99;

  B2DEBUG(35, "ECLDigitCalibratorModule::getCalibratedTimeResolution: dose = " << m_th1fBackground->GetBinContent(
            cellid) << ", bglevel = " << bglevel << ", cellid = " << cellid << ", t99 = " << t99 << ", energy = " << energy /
          Belle2::Unit::MeV);

  return t99;
}

// Determine background level by event by counting out-of-time digits above threshold.
int ECLDigitCalibratorModule::determineBackgroundECL()
{
  //EventLevelClustering counters
  using regionCounter = std::unordered_map<ECL::DetectorRegion, uint>;

  regionCounter outOfTimeCount {{ECL::DetectorRegion::FWD, 0},
    {ECL::DetectorRegion::BRL, 0},
    {ECL::DetectorRegion::BWD, 0}};

  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  // Loop over the input array
  for (auto& aECLCalDigit : m_eclCalDigits) {
    if (abs(aECLCalDigit.getTime()) >= m_backgroundTimingCut) {
      if (aECLCalDigit.getEnergy() >= m_backgroundEnergyCut) {
        //Get digit theta
        const B2Vector3D position  = geom->GetCrystalPos(aECLCalDigit.getCellId() - 1);
        const double theta         = position.Theta();

        //Get detector region
        const auto detectorRegion = ECL::getDetectorRegion(theta);

        //Count out of time digits per region
        ++outOfTimeCount.at(detectorRegion);
      }
    }
  }

  //Save EventLevelClusteringInfo
  if (!m_eventLevelClusteringInfo) {
    m_eventLevelClusteringInfo.create();
  }

  m_eventLevelClusteringInfo->setNECLCalDigitsOutOfTimeFWD(outOfTimeCount.at(ECL::DetectorRegion::FWD));
  m_eventLevelClusteringInfo->setNECLCalDigitsOutOfTimeBarrel(outOfTimeCount.at(ECL::DetectorRegion::BRL));
  m_eventLevelClusteringInfo->setNECLCalDigitsOutOfTimeBWD(outOfTimeCount.at(ECL::DetectorRegion::BWD));

  B2DEBUG(35, "ECLDigitCalibratorModule::determineBackgroundECL found " << outOfTimeCount.at(ECL::DetectorRegion::FWD) << ", " <<
          outOfTimeCount.at(ECL::DetectorRegion::BRL) << ", " <<
          outOfTimeCount.at(ECL::DetectorRegion::BWD) << " out of time digits in FWD, BRL, BWD");

  return m_eventLevelClusteringInfo->getNECLCalDigitsOutOfTime();

}


