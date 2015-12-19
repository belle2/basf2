/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/modules/eclDigitCalibration/ECLDigitCalibratorModule.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>

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
ECLDigitCalibratorModule::ECLDigitCalibratorModule() : m_calibrationLow("ECLCalibrationDigitLow"),
  m_calibrationHigh("ECLCalibrationDigitHigh")
{
  // Set module properties
  setDescription("Applies digit calibration to each ECL digit.");

  // Parallel processing
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

  // Register Digits, CalDigits and their relation in datastore
  eclDigits.registerInDataStore();
  eclCalDigits.registerInDataStore();
  eclCalDigits.registerRelationTo(eclDigits);

  // resize vector that holds calibration amplitudes A, energies E and constants c0 and c1
  m_calibrationAmplitudesLowEnergy.resize(c_nCrystals + 1); //+1 is to match the cellid[1..] convention later
  m_calibrationEnergiesLowEnergy.resize(c_nCrystals + 1);
  m_calibrationAmplitudesHighEnergy.resize(c_nCrystals + 1);
  m_calibrationEnergiesHighEnergy.resize(c_nCrystals + 1);
  m_calibrationC0.resize(c_nCrystals + 1);
  m_calibrationC1.resize(c_nCrystals + 1);

}

// begin run
void ECLDigitCalibratorModule::beginRun()
{
  // get the calibration objects and put them into vectors (to be accessed via cell id later)
  prepareCalibrationConstants();

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

    // perform the digit energy calibration
    const int amplitude           = aECLDigit.getAmp();
    const double calibratedEnergy = getCalibratedEnergy(cellid, amplitude); // Fitted amplitude to GeV

    // perform the digit timing calibration
    const int time              = aECLDigit.getTimeFit();
    const double calibratedTime = getCalibratedTime(cellid, time); //

    // fill the ECLCalDigit with the cell id, the calibrated information and calibration status
    aECLCalDigit->setCellId(cellid);
    aECLCalDigit->setEnergy(calibratedEnergy);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsEnergyCalibrated);
    aECLCalDigit->setTime(calibratedTime);
    aECLCalDigit->addStatus(ECLCalDigit::c_IsTimeCalibrated);

    // set a relation to the ECLDigit
    aECLCalDigit->addRelationTo(&aECLDigit);
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

// energy calibration
double ECLDigitCalibratorModule::getCalibratedEnergy(int cellid, int amplitude)
{
  if (cellid < 1 || cellid > c_nCrystals) {
    B2FATAL("ECLDigitCalibrationModule:getCalibratedEnergy():" << cellid << " out od range!");
  }

  //get sign of amplitude (can be negative as result of the fit!)
  int sign = 1;
  if (amplitude < 0) {
    amplitude = -amplitude; //will give the correct answer on average
    sign = -1;
  }

  //protect the logarithm
  if (amplitude <= 1) return 0.;

  // get the two energy points and respective calibration constants for this crystal as in B2N-XXX (tba)
  const float c0  = m_calibrationC0.at(cellid);
  const float c1  = m_calibrationC1.at(cellid);
  double calenergy = c0 * (1.0 + c1 * std::log(static_cast<double>(amplitude))) * static_cast<double>(amplitude);

  // get the correct sign back
  calenergy *= sign;

  return calenergy;
}

// time calibration (skeleton only)
double ECLDigitCalibratorModule::getCalibratedTime(int cellid, int time)
{
  if (cellid < 1 || cellid > c_nCrystals) {
    B2FATAL("ECLDigitCalibrationModule:getCalibratedTime():" << cellid << " out od range!");
  }

  double caltime = 1.0 * time;

  return caltime;
}

// prepare energy and time calibration constants
void ECLDigitCalibratorModule::prepareCalibrationConstants()
{
  // low energy constants
  std::fill(m_calibrationAmplitudesLowEnergy.begin(), m_calibrationAmplitudesLowEnergy.end(), 0.);
  std::fill(m_calibrationEnergiesLowEnergy.begin(), m_calibrationEnergiesLowEnergy.end(), 0.);
  int nCalibrationLowEnergy = 0;

  for (const ECLCalibrationDigit& calLow : m_calibrationLow) {
    const int cellid      = calLow.getCellID();
    const float amplitude = calLow.getAmplitude();
    const float energy    = calLow.getEnergy();

    if (cellid < 1 || cellid > c_nCrystals) { // check if the cellid is withing range
      B2ERROR("[ECLDigitCalibratorModule::prepareCalibrationConstants()], low energy: Wrong cell id [0.." << c_nCrystals << "]: " <<
              cellid);
      continue;
    } else if (m_calibrationEnergiesLowEnergy[cellid] > 1e-5) { //FIXME magic number //check if the field is already filled
      B2WARNING("[ECLDigitCalibratorModule::prepareCalibrationConstants()], low energy: Constants for cell id " << cellid <<
                " already filled");
    } else { // basic checks passed, fill the vector entry
      m_calibrationAmplitudesLowEnergy[cellid] = amplitude;
      m_calibrationEnergiesLowEnergy[cellid] = energy;
      nCalibrationLowEnergy++;
    }
  }

  // check if all cell ids have a calibration amplitude
  if (nCalibrationLowEnergy != c_nCrystals) {
    B2FATAL("[ECLDigitCalibratorModule::prepareCalibrationConstants()], low energy: Not all cells have valid low energy calibration constants!");
  }

  // high energy constants
  std::fill(m_calibrationAmplitudesHighEnergy.begin(), m_calibrationAmplitudesHighEnergy.end(), 0.);
  std::fill(m_calibrationEnergiesHighEnergy.begin(), m_calibrationEnergiesHighEnergy.end(), 0.);
  int nCalibrationHighEnergy = 0;

  for (const ECLCalibrationDigit& calHigh : m_calibrationHigh) {
    const int cellid      = calHigh.getCellID();
    const float amplitude = calHigh.getAmplitude();
    const float energy    = calHigh.getEnergy();

    if (cellid < 1 || cellid > c_nCrystals) { // check if the cellid is withing range
      B2ERROR("[ECLDigitCalibratorModule::prepareCalibrationConstants()], high energy: Wrong cell id [0.." << c_nCrystals << "]: " <<
              cellid);
      continue;
    } else if (m_calibrationEnergiesHighEnergy[cellid] > 1e-5) { //FIXME magic number //check if the field is already filled
      B2WARNING("[ECLDigitCalibratorModule::prepareCalibrationConstants()], high energy: Constants for cell id " << cellid <<
                " already filled");
    } else { // basic checks passed, fill the vector entry
      m_calibrationAmplitudesHighEnergy[cellid] = amplitude;
      m_calibrationEnergiesHighEnergy[cellid] = energy;
      nCalibrationHighEnergy++;
    }
  }

  // check if all cell ids have a calibration constant
  if (nCalibrationHighEnergy != c_nCrystals) {
    B2FATAL("[ECLDigitCalibratorModule::prepareCalibrationConstants()], high energy: Not all cells have valid high energy calibration constants!");
  }

  // calculate the constants c0 and c1 and store them in the vector
  for (int i = 1; i <= c_nCrystals; ++i) {
    const double elow  = m_calibrationEnergiesLowEnergy[i];
    const double alow  = m_calibrationAmplitudesLowEnergy[i];
    const double ehigh = m_calibrationEnergiesHighEnergy[i];
    const double ahigh = m_calibrationAmplitudesHighEnergy[i];

    if (elow < 1e-6 || ehigh < 1e-6) {
      B2FATAL("[ECLDigitCalibratorModule::prepareCalibrationConstants()], too small calibration energies!");
    }

    const double glow  = alow / elow;
    const double ghigh = ahigh / ehigh;

    // equations for log(E) dependent calibration, solutions of linear equation system:
    m_calibrationC0[i] = (ghigh * std::log(ahigh) - glow * std::log(alow)) / (glow * ghigh * (std::log(ahigh) - std::log(alow)));
    m_calibrationC1[i] = (glow - ghigh) / (ghigh * std::log(ahigh) - glow * std::log(alow));
  }
}
