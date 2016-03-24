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

// FRAMEWORK
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

// DATABASE
#include <framework/conditions/ConditionsService.h>

// SYSTEM
#include <ctime>
#include <iomanip>

// ROOT
#include <TVector3.h>
#include <TMatrixFSym.h>
#include <TMath.h>
#include <TFile.h>
#include <TTree.h>

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
ECLDigitCalibratorModule::ECLDigitCalibratorModule() : m_calibrationEnergyHigh("ECLDigitEnergyConstantsHigh"),
  m_calibrationTime("ECLDigitTimeConstants")
{
  // Set module properties
  setDescription("Applies digit energy, time and time-resolution calibration to each ECL digit. Counts number of out-of-time background digits to determine the event-by-event background level.");
  addParam("backgroundEnergyCut", m_backgroundEnergyCut, "Energy cut used to count background digits", 7.0 * Belle2::Unit::MeV);
  addParam("backgroundTimingCut", m_backgroundTimingCut, "Timing cut used to count background digits", 150.0 * Belle2::Unit::ns);

  // Parallel processing certification
  setPropertyFlags(c_ParallelProcessingCertified);

}

// destructor
ECLDigitCalibratorModule::~ECLDigitCalibratorModule()
{
}

// initialize
void ECLDigitCalibratorModule::initialize()
{
  // ECL dataobjects
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreObjPtr<ECLEventInformation> eclEventInformationPtr(eclEventInformationName());

  // Register Digits, CalDigits and their relation in datastore
  eclDigits.registerInDataStore();
  eclCalDigits.registerInDataStore();
  eclCalDigits.registerRelationTo(eclDigits);
  eclEventInformationPtr.registerInDataStore();

  // resize vector that holds calibration amplitudes A, energies E and constants c0 and c1
  m_calibrationEnergyAmplitudeHigh.resize(c_nCrystals + 1);
  m_calibrationEnergyEnergyHigh.resize(c_nCrystals + 1);

  // resize vector that holds the time (and time resolution) calibration constants
  m_calibrationTimeOffset.resize(c_nCrystals + 1);

  // background information (cuts can be reset via module parameters)
  m_backgroundEnergyCut = 7.0 * Belle2::Unit::MeV;  /**< Energy cut for background level counting. */
  m_backgroundTimingCut = 150.0 * Belle2::Unit::ns;  /**< Timing window for background level counting. */

  // HARDCODED VALUES WILL DISAPPEAR FOR RELEASE-00-08
  // time calibration for MC: t = a * (m_timeFit + b)
  m_timeInverseSlope = 1.0 / 2.0366; // "b", (CH for svn revision 25745)

  // time resolution calibration for MC (for full background. for no background, this will be a pessimistic approximation)
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
  // get the calibration objects and put them into vectors (to be accessed via cell id later)
  prepareEnergyCalibrationConstants();
  prepareTimeCalibrationConstants();

}

// event
void ECLDigitCalibratorModule::event()
{
  // Input Array(s)
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());

  // Output Array(s)
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());

  // Loop over the input array
  for (auto& aECLDigit : eclDigits) {

    // create eclCalDigits if they dont exist already
    if (!eclCalDigits) eclCalDigits.create();

    // append an ECLCalDigit to the storearray
    const auto aECLCalDigit = eclCalDigits.appendNew();

    // get the cell id from the ECLDigit as identifier
    // FIXME: could be removed, cellid can be obtained via relation
    const int cellid = aECLDigit.getCellId();

    // check that the cell id is valid
    if (cellid < 1 || cellid > c_nCrystals) {
      B2FATAL("ECLDigitCalibrationModule::event():" << cellid << " out of range!");
    }

    // perform the digit energy calibration
    const int amplitude           = aECLDigit.getAmp();
    const double calibratedEnergy = getCalibratedEnergy(cellid, amplitude); // Fitted amplitude to GeV

    // perform the digit timing calibration
    const int time              = aECLDigit.getTimeFit();
    if (time == -2048) aECLCalDigit->addStatus(ECLCalDigit::c_IsFailedFit); //this is used to flag failed fits
    const double calibratedTime = getCalibratedTime(cellid, time,
                                                    aECLCalDigit->hasStatus(ECLCalDigit::c_IsFailedFit)); // calibrated time

    // perform the time resolution calibration
    const double calibratedTimeResolution = getCalibratedTimeResolution(cellid, calibratedEnergy,
                                            aECLCalDigit->hasStatus(ECLCalDigit::c_IsFailedFit)); // calibrated time resolution

    // fill the ECLCalDigit with the cell id, the calibrated information and calibration status
    aECLCalDigit->setCellId(cellid);
    aECLCalDigit->setEnergy(calibratedEnergy);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsEnergyCalibrated);

    aECLCalDigit->setTime(calibratedTime);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsTimeCalibrated);

    aECLCalDigit->setTimeResolution(calibratedTimeResolution);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsTimeResolutionCalibrated);

    // set a relation to the ECLDigit
    aECLCalDigit->addRelationTo(&aECLDigit);

    B2DEBUG(175, "ECLDigitCalibratorModule: Time (raw): " << aECLDigit.getTimeFit() << ", Time (calibrated): " <<
            aECLCalDigit->getTime() << ", Time resolution (calibrated): " << aECLCalDigit->getTimeResolution());
    B2DEBUG(175, "ECLDigitCalibratorModule: Amplitude (raw): " << aECLDigit.getAmp() << ", Energy (calibrated): " <<
            aECLCalDigit->getEnergy());
  }

  // determine background level
  determineBackgroundECL();

}

// end run
void ECLDigitCalibratorModule::endRun()
{

}

// terminate
void ECLDigitCalibratorModule::terminate()
{

}

// Energy calibration
double ECLDigitCalibratorModule::getCalibratedEnergy(int cellid, int amplitude)
{
  //get sign of amplitude (could be negative as result of the fit if we would have implemented that in hardware )
  int sign = 1;
  if (amplitude < 0) {
    amplitude = -amplitude; //will give the correct answer on average
    sign = -1;
  }

  //for small amplitudes we return zero (will protect the log as well)
  if (amplitude <= c_MinimumAmplitude) return c_energyForSmallAmplitude;

  // get the two energy points and respective calibration constants for this crystal E = A * calA/calE
  const float calA  = m_calibrationEnergyAmplitudeHigh.at(cellid);
  const float calE  = m_calibrationEnergyEnergyHigh.at(cellid);
  B2DEBUG(175, "ECLDigitCalibratorModule::getCalibratedEnergy: calA = " << calA << ", calE = " << calE);

  double calenergy = static_cast<double>(amplitude) * calE / calA;

  // get the correct sign back
  calenergy *= sign;

  return calenergy;
}

// Time calibration
// this will eventually depend on the cell id for data - as of release-00-07 it is the same for all crystals.
double ECLDigitCalibratorModule::getCalibratedTime(const int cellid, const int fittedtime, const bool fitfailed)
{

  double caltime = c_timeForFitFailed; // Set the time to zero ("trigger time") in case the waveform fit failed
  if (!fitfailed) {
    caltime = m_timeInverseSlope * (fittedtime + m_calibrationTimeOffset[cellid] * Belle2::Unit::ns);  // calibrated time from fit
    B2DEBUG(175, "ECLDigitCalibratorModule::getCalibratedTime: timeInverseSlope = " << m_timeInverseSlope <<
            ", calibrationTimeOffset [ns] = " << m_calibrationTimeOffset[cellid] * Belle2::Unit::ns);

  }

  return caltime;
}

// Time resolution calibration
double ECLDigitCalibratorModule::getCalibratedTimeResolution(const int cellid, const double energy, const bool fitfailed)
{

  // if this digit is calibrated to the trigger time, we set the resolution to 'very bad'
  if (fitfailed) return c_timeResolutionForFitFailed;

  // piecewise linear extrapolation in x with E (in GeV) = 1/x
  double x = 1.e12; //
  if (energy > 1.e-9) x = 1. / (energy / Belle2::Unit::GeV);  // avoid division by (almost) zero
  else return c_timeResolutionForZeroEnergy;

  int bin = 0;
  if (x > m_timeResolutionPointX[2]) bin = 2;
  else if (x > m_timeResolutionPointX[1]) bin = 1;

  double timeresolution = getInterpolatedTimeResolution(x, bin);

  return timeresolution;
}

// time resolution interpolation
double ECLDigitCalibratorModule::getInterpolatedTimeResolution(const double x, const int bin)
{
  double interpolation = m_timeResolutionPointResolution[bin] +
                         (m_timeResolutionPointResolution[bin + 1] - m_timeResolutionPointResolution[bin]) *
                         (x - m_timeResolutionPointX[bin]) / (m_timeResolutionPointX[bin + 1] - m_timeResolutionPointX[bin]);

  return interpolation;
}

// prepare energy calibration constants
void ECLDigitCalibratorModule::prepareEnergyCalibrationConstants()
{

  // High energy ENERGY CALIBRATION
  std::fill(m_calibrationEnergyAmplitudeHigh.begin(), m_calibrationEnergyAmplitudeHigh.end(), 0.);
  std::fill(m_calibrationEnergyEnergyHigh.begin(), m_calibrationEnergyEnergyHigh.end(), 0.);
  int nCalibrationHighEnergy = 0;

  for (const ECLDigitEnergyConstants& calHigh : m_calibrationEnergyHigh) {
    const int cellid      = calHigh.getCellID();
    const float amplitude = calHigh.getAmplitude();
    const float energy    = calHigh.getEnergy();

    if (cellid < 1 || cellid > c_nCrystals) { // check if the cellid is withing range
      B2ERROR("ECLDigitCalibratorModule::prepareEnergyCalibrationConstants(): high energy: Wrong cell id [0.." << c_nCrystals << "]: " <<
              cellid);
      continue;
    } else if (m_calibrationEnergyEnergyHigh[cellid] > 1e-5) { //FIXME magic number //check if the field is already filled
      B2WARNING("ECLDigitCalibratorModule::prepareEnergyCalibrationConstants(): high energy: Constants for cell id " << cellid <<
                " already filled");
    } else { // basic checks passed, fill the vector entry
      m_calibrationEnergyAmplitudeHigh[cellid] = amplitude;
      m_calibrationEnergyEnergyHigh[cellid] = energy;
      nCalibrationHighEnergy++;
    }
  }

  // check if all cell ids have a calibration constant
  if (nCalibrationHighEnergy != c_nCrystals) {
    B2FATAL("[ECLDigitCalibratorModule::prepareEnergyCalibrationConstants()], high energy: Not all cells have valid high energy calibration constants!");
  }

}


// prepare time calibration constants
void ECLDigitCalibratorModule::prepareTimeCalibrationConstants()
{

  // TIME CALIBRATION
  std::fill(m_calibrationTimeOffset.begin(), m_calibrationTimeOffset.end(), 0.);
  int nCalibrationTime = 0;

  for (const ECLDigitTimeConstants& calTime : m_calibrationTime) {
    const int cellid   = calTime.getCellID();
    const float offset = calTime.getOffset();

    if (cellid < 1 || cellid > c_nCrystals) { // check if the cellid is withing range
      B2ERROR("ECLDigitCalibratorModule::prepareTimeCalibrationConstants(): high energy: Wrong cell id [0.." << c_nCrystals << "]: " <<
              cellid);
      continue;
    } else { // basic checks passed, fill the vector entry
      m_calibrationTimeOffset[cellid] = offset;
      nCalibrationTime++;
    }
  }

  // check if all cell ids have a calibration constant
  if (nCalibrationTime != c_nCrystals) {
    B2FATAL("[ECLDigitCalibratorModule::prepareTimeCalibrationConstants()], time: Not all cells have valid time calibration constants!");
  }

}

// determine background level by event by counting out-of-time digits above threshold
void ECLDigitCalibratorModule::determineBackgroundECL()
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

  // if an event misses the ECL we will have zero digits in total or we have another problem,
  // set background level to -1 to indicate true zero ECL hits (even below cuts)
  if (totalcount < 0) backgroundcount = -1;

  // put into EventInformation dataobject
  if (!eclEventInformationPtr) eclEventInformationPtr.create();
  eclEventInformationPtr->setBackgroundECL(backgroundcount);

}
