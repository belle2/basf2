/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Digit Calibration.                                                     *
 *                                                                        *
 * This  module converts the fitted amplitude into calibrated energy,     *
 * fitted time into calibrated time, and determines the time resolution   *
 * per digit. It furthermore determines the background level by counting  *
 * out of time digits above a certain energy threshold.                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Chris Hearty (hearty@physics.ubc.ca) (CH)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/modules/eclDigitCalibration/ECLDigitCalibratorModule.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLEventInformation.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/digitization/EclConfigurationPure.h>
#include <ecl/dataobjects/ECLPureCsIInfo.h>
#include <ecl/dataobjects/ECLDsp.h>

// FRAMEWORK
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

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
  m_calibrationCrystalFlightTime("ECLCrystalFlightTime")
{
  // Set module properties
  setDescription("Applies digit energy, time and time-resolution calibration to each ECL digit. Counts number of out-of-time background digits to determine the event-by-event background level.");
  addParam("backgroundEnergyCut", m_backgroundEnergyCut, "Energy cut used to count background digits", 7.0 * Belle2::Unit::MeV);
  addParam("backgroundTimingCut", m_backgroundTimingCut, "Timing cut used to count background digits", 110.0 * Belle2::Unit::ns);
  addParam("fileBackgroundName", m_fileBackgroundName, "Background filename.",
           FileSystem::findFile("/data/ecl/background_norm.root"));
  addParam("simulatePure", m_simulatePure, "Flag to simulate pure CsI option", false);

  // Parallel processing certification
  setPropertyFlags(c_ParallelProcessingCertified);

}

// destructor
ECLDigitCalibratorModule::~ECLDigitCalibratorModule()
{
  ;
}

// initialize calibration
void ECLDigitCalibratorModule::initializeCalibration()
{

  m_timeInverseSlope = 1.0 / (4.0 * EclConfiguration::m_rf) *
                       1e3;  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz. Same for all crystals.
  m_pureCsIEnergyCalib = 0.00005; //conversion factor from ADC counts to GeV
  m_pureCsITimeCalib = 0.1; //conversion factor from eclPureCsIDigitizer to ns
  m_pureCsITimeOffset = 0.31; //ad-hoc offset correction for pureCsI timing

  callbackCalibration(m_calibrationCrystalElectronics, v_calibrationCrystalElectronics, v_calibrationCrystalElectronicsUnc);
  callbackCalibration(m_calibrationCrystalEnergy, v_calibrationCrystalEnergy, v_calibrationCrystalEnergyUnc);
  callbackCalibration(m_calibrationCrystalElectronicsTime, v_calibrationCrystalElectronicsTime,
                      v_calibrationCrystalElectronicsTimeUnc);
  callbackCalibration(m_calibrationCrystalTimeOffset, v_calibrationCrystalTimeOffset, v_calibrationCrystalTimeOffsetUnc);
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
  // ECL dataobjects
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());
  StoreObjPtr<ECLEventInformation> eclEventInformationPtr(eclEventInformationName());

  // Register Digits, CalDigits and their relation in datastore
  eclDigits.registerInDataStore(eclDigitArrayName());
  eclDsps.registerInDataStore(eclDspArrayName());
  eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  eclCalDigits.registerRelationTo(eclDigits);
  eclEventInformationPtr.registerInDataStore(eclEventInformationName());

  //Special information for pure CsI simulation
  StoreArray<ECLPureCsIInfo> eclPureCsIInfo(eclPureCsIInfoArrayName());
  eclPureCsIInfo.registerInDataStore(eclPureCsIInfoArrayName());

  // initialize calibration
  initializeCalibration();

  // initialize time resolution (not yet from the database)
  // read the Background correction factors (for full background)
  m_fileBackground = new TFile(m_fileBackgroundName.c_str(), "READ");
  if (!m_fileBackground) B2FATAL("Could not find file: " << m_fileBackgroundName);
  m_th1dBackground = (TH1D*) m_fileBackground->Get("background");

  // average BG value from m_th1dBackground
  m_averageBG = m_th1dBackground->Integral() / m_th1dBackground->GetEntries();

  // get maximum position ("x") of 2-order pol background for t99
  if (fabs(c_pol2Var3) > 1e-12) {
    m_pol2Max = -c_pol2Var2 / (2 * c_pol2Var3);
  } else {
    m_pol2Max = 0.;
  }

  // time resolution calibration for MC (for full background. for no background, this will be a pessimistic approximation.)
  m_timeResolutionPointResolution[0] =   0.134 * Belle2::Unit::ns; // (CH for svn revision 26660)
  m_timeResolutionPointResolution[1] =  12.23 * Belle2::Unit::ns; // (CH for svn revision 26660)
  m_timeResolutionPointResolution[2] =  85.74 * Belle2::Unit::ns; // (CH for svn revision 26660)
  m_timeResolutionPointResolution[3] = 342.9 * Belle2::Unit::ns; // (CH for svn revision 26660)
  m_timeResolutionPointX[0] =   0.0; // (CH for svn revision 26660)
  m_timeResolutionPointX[1] =  15.65; // (CH for svn revision 26660)
  m_timeResolutionPointX[2] = 109.0; // (CH for svn revision 26660)
  m_timeResolutionPointX[3] = 400.0; // (CH for svn revision 26660)

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

  if (m_calibrationCrystalFlightTime.hasChanged()) {
    if (m_calibrationCrystalFlightTime) {
      callbackCalibration(m_calibrationCrystalFlightTime, v_calibrationCrystalFlightTime, v_calibrationCrystalFlightTimeUnc);
    } else B2ERROR("ECLDigitCalibratorModule::beginRun - Couldn't find m_calibrationCrystalFlightTime for current run!");
  }

}

// event
void ECLDigitCalibratorModule::event()
{
  // Input Array(s)
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());

  StoreArray<ECLDsp> eclDsps(eclDspArrayName());

  // Output Array(s)
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());

  // Special Array for Pure CsI Simulation
  StoreArray<ECLPureCsIInfo> eclPureCsIInfo(eclPureCsIInfoArrayName());

  // Loop over the input array
  for (auto& aECLDigit : eclDigits) {

    // create eclCalDigits if they dont exist already

    bool is_pure_csi = 0;

    // append an ECLCalDigit to the storearray
    const auto aECLCalDigit = eclCalDigits.appendNew();

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
      if (aECLDigit.getRelated<ECLPureCsIInfo>(eclPureCsIInfoArrayName()) != NULL) {
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
    double calibratedTime = 0;
    if (time == -2048) {
      aECLCalDigit->addStatus(ECLCalDigit::c_IsFailedFit); //this is used to flag failed fits
    } else { //only calibrate digit time if we have a good waveform fit
      if (is_pure_csi) {
        calibratedTime = m_pureCsITimeCalib * m_timeInverseSlope * (time - v_calibrationCrystalElectronicsTime[cellid - 1] -
                                                                    v_calibrationCrystalTimeOffset[cellid - 1]) - v_calibrationCrystalFlightTime[cellid - 1] + m_pureCsITimeOffset;
      } else {
        calibratedTime = m_timeInverseSlope * (time - v_calibrationCrystalElectronicsTime[cellid - 1] -
                                               v_calibrationCrystalTimeOffset[cellid - 1]) - v_calibrationCrystalFlightTime[cellid - 1];
      }
    }

    B2DEBUG(175, "cellid = " << cellid << ", amplitude = " << amplitude << ", calibrated energy = " << calibratedEnergy);
    B2DEBUG(175, "cellid = " << cellid << ", time = " << time << ", calibratedTime = " << calibratedTime);

    //Calibrating offline fit results
    ECLDsp* aECLDsp = aECLDigit.getRelatedFrom<ECLDsp>();
    aECLCalDigit->setTwoComponentChi2(-1);
    aECLCalDigit->setTwoComponentTotalEnergy(-1);
    aECLCalDigit->setTwoComponentHadronEnergy(-1);
    if (aECLDsp) {
      //require ECLDigit to have offline waveform
      if (aECLDsp->getTwoComponentChi2() > 0) {
        //require offline waveform to have offline fit result
        //
        double calibratedTwoComponentTotalEnergy = aECLDsp->getTwoComponentTotalAmp() * v_calibrationCrystalElectronics[cellid - 1] *
                                                   v_calibrationCrystalEnergy[cellid - 1];
        double calibratedTwoComponentHadronEnergy = aECLDsp->getTwoComponentHadronAmp() * v_calibrationCrystalElectronics[cellid - 1] *
                                                    v_calibrationCrystalEnergy[cellid - 1];
        double twoComponentChi2 = aECLDsp->getTwoComponentChi2();
        //
        aECLCalDigit->setTwoComponentTotalEnergy(calibratedTwoComponentTotalEnergy);
        aECLCalDigit->setTwoComponentHadronEnergy(calibratedTwoComponentHadronEnergy);
        aECLCalDigit->setTwoComponentChi2(twoComponentChi2);
        //
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
  for (auto& aECLCalDigit : eclCalDigits) {

    // perform the time resolution calibration
    const double t99 = getT99(aECLCalDigit.getCellId(),
                              aECLCalDigit.getEnergy(),
                              aECLCalDigit.hasStatus(ECLCalDigit::c_IsFailedFit),
                              bgCount); // calibrated time resolution
    aECLCalDigit.setTimeResolution(t99);
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
  const double bglevel = TMath::Min((double) bgcount / (double) c_nominalBG * m_th1dBackground->GetBinContent(cellid) / m_averageBG,
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

//  // piecewise linear extrapolation in x with E (in GeV) = 1/x
//  double x = 1.e12; //
//  if (energy > 1.e-9) x = 1. / (energy / Belle2::Unit::GeV);  // avoid division by (almost) zero
//  else return c_timeResolutionForZeroEnergy;
//
//  int bin = 0;
//  if (x > m_timeResolutionPointX[2]) bin = 2;
//  else if (x > m_timeResolutionPointX[1]) bin = 1;
//
//  double timeresolution = getInterpolatedTimeResolution(x, bin);

  B2DEBUG(175, "ECLDigitCalibratorModule::getCalibratedTimeResolution: dose = " << m_th1dBackground->GetBinContent(
            cellid) << ", bglevel = " << bglevel << ", cellid = " << cellid << ", t99 = " << t99 << ", energy = " << energy /
          Belle2::Unit::MeV);

  return t99;
}

// Determine background level by event by counting out-of-time digits above threshold.
int ECLDigitCalibratorModule::determineBackgroundECL()
{
  // Input StoreObjArray(s)
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());

  // Output StoreObj
  StoreObjPtr<ECLEventInformation> eclEventInformationPtr(eclEventInformationName());

  int backgroundcount = 0;
  int totalcount = 0;
  // Loop over the input array
  for (auto& aECLCalDigit : eclCalDigits) {
    if (abs(aECLCalDigit.getTime()) >= m_backgroundTimingCut) {
      if (aECLCalDigit.getEnergy() >= m_backgroundEnergyCut) {
        ++backgroundcount;
      }
    }
    ++totalcount;
  }

  // If an event misses the ECL we will have zero digits in total or we have another problem,
  // set background level to -1 to indicate true zero ECL hits (even below cuts).
  if (totalcount == 0) backgroundcount = -1;

  // Put into EventInformation dataobject.
  if (!eclEventInformationPtr) eclEventInformationPtr.create();
  eclEventInformationPtr->setBackgroundECL(backgroundcount);

  B2DEBUG(175, "ECLDigitCalibratorModule::determineBackgroundECL(): backgroundcount = " << backgroundcount);
  return backgroundcount;

}
