/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclMuMuECollector/eclMuMuECollectorModule.h>

//ROOT
#include <TH2F.h>

//Framework
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>

//Tracking
#include <tracking/dataobjects/ExtHit.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dataobjects/ECLDigit.h>

//MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TRGSummary.h>

using namespace std;
using namespace Belle2;
using namespace ECL;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclMuMuECollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

eclMuMuECollectorModule::eclMuMuECollectorModule() : CalibrationCollectorModule(), m_ECLExpMuMuE("ECLExpMuMuE"),
  m_ElectronicsCalib("ECLCrystalElectronics"), m_MuMuECalib("ECLCrystalEnergyMuMu")
{
  // Set module properties
  setDescription("Calibration Collector Module for ECL single crystal energy calibration using muons");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("minPairMass", m_minPairMass, "minimum invariant mass of the muon pair (GeV/c^2)", 9.0);
  addParam("minTrackLength", m_minTrackLength, "minimum extrapolated track length in the crystal (cm)", 30.);
  addParam("MaxNeighbourE", m_MaxNeighbourE, "maximum energy allowed in a neighbouring crystal (GeV)", 0.010);
  addParam("thetaLabMinDeg", m_thetaLabMinDeg, "miniumum muon theta in lab (degrees)", 17.);
  addParam("thetaLabMaxDeg", m_thetaLabMaxDeg, "maximum muon theta in lab (degrees)", 150.);
  addParam("measureTrueEnergy", m_measureTrueEnergy, "use MC events to obtain expected energies", false);
  addParam("requireL1", m_requireL1, "only use events that have a level 1 trigger", true);
}

/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclMuMuECollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  /** MetaData */

  B2INFO("eclMuMuECollector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto EnVsCrysID = new TH2F("EnVsCrysID", "Normalized energy for each crystal;crystal ID;E/Expected", 8736, 0, 8736, 120, 0.1, 2.5);
  registerObject<TH2F>("EnVsCrysID", EnVsCrysID);

  auto ExpEvsCrys = new TH1F("ExpEvsCrys", "Sum expected energy vs crystal ID;crystal ID;Energy (GeV)", 8736, 0, 8736);
  registerObject<TH1F>("ExpEvsCrys", ExpEvsCrys);

  auto ElecCalibvsCrys = new TH1F("ElecCalibvsCrys", "Sum electronics calib const vs crystal ID;crystal ID;calibration constant",
                                  8736, 0, 8736);
  registerObject<TH1F>("ElecCalibvsCrys", ElecCalibvsCrys);

  auto InitialCalibvsCrys = new TH1F("InitialCalibvsCrys",
                                     "Sum initial muon pair calib const vs crystal ID;crystal ID;calibration constant", 8736, 0, 8736);
  registerObject<TH1F>("InitialCalibvsCrys", InitialCalibvsCrys);

  auto CalibEntriesvsCrys = new TH1F("CalibEntriesvsCrys", "Entries in calib vs crys histograms;crystal ID;Entries per crystal", 8736,
                                     0, 8736);
  registerObject<TH1F>("CalibEntriesvsCrys", CalibEntriesvsCrys);

  /** Raw digit quantities for debugging purposes only */
  auto RawDigitAmpvsCrys = new TH2F("RawDigitAmpvsCrys", "Digit Amplitude vs crystal ID;crystal ID;Amplitude", 8736, 0, 8736, 250, 0,
                                    25000);
  registerObject<TH2F>("RawDigitAmpvsCrys", RawDigitAmpvsCrys);

  auto RawDigitTimevsCrys = new TH2F("RawDigitTimevsCrys", "Digit Time vs crystal ID;crystal ID;Time", 8736, 0, 8736, 200, -2000,
                                     2000);
  registerObject<TH2F>("RawDigitTimevsCrys", RawDigitTimevsCrys);


  //------------------------------------------------------------------------
  /** Four or ~eight nearest neighbours, plus crystal itself. ECLNeighbour uses cellID, 1--8736 */
  myNeighbours4 = new ECLNeighbours("NC", 1);
  myNeighbours8 = new ECLNeighbours("N", 1);


  //------------------------------------------------------------------------
  /** Parameters */
  B2INFO("Input parameters to eclMuMuECollector:");
  B2INFO("minPairMass: " << m_minPairMass);
  B2INFO("minTrackLength: " << m_minTrackLength);
  B2INFO("MaxNeighbourE: " << m_MaxNeighbourE);
  B2INFO("thetaLabMinDeg: " << m_thetaLabMinDeg);
  B2INFO("thetaLabMaxDeg: " << m_thetaLabMaxDeg);
  if (m_thetaLabMinDeg < 1.) {
    cotThetaLabMax = 9999.;
  } else if (m_thetaLabMinDeg > 179.) {
    cotThetaLabMax = -9999.;
  } else {
    double thetaLabMin = m_thetaLabMinDeg * TMath::Pi() / 180.;
    cotThetaLabMax = 1. / tan(thetaLabMin);
  }
  if (m_thetaLabMaxDeg < 1.) {
    cotThetaLabMin = 9999.;
  } else if (m_thetaLabMaxDeg > 179.) {
    cotThetaLabMin = -9999.;
  } else {
    double thetaLabMax = m_thetaLabMaxDeg * TMath::Pi() / 180.;
    cotThetaLabMin = 1. / tan(thetaLabMax);
  }
  B2INFO("cotThetaLabMin: " << cotThetaLabMin);
  B2INFO("cotThetaLabMax: " << cotThetaLabMax);
  B2INFO("measureTrueEnergy: " << m_measureTrueEnergy);
  B2INFO("requireL1: " << m_requireL1);

  /** Resize vectors */
  EperCrys.resize(8736);

  /**----------------------------------------------------------------------------------------*/
  /** Get expected energies and calibration constants from DB. Need to call hasChanged() for later comparison */
  if (m_ECLExpMuMuE.hasChanged()) {ExpMuMuE = m_ECLExpMuMuE->getCalibVector();}
  if (m_ElectronicsCalib.hasChanged()) {ElectronicsCalib = m_ElectronicsCalib->getCalibVector();}
  if (m_MuMuECalib.hasChanged()) {MuMuECalib = m_MuMuECalib->getCalibVector();}

  /** Write out a few for quality control */
  for (int ic = 1; ic < 9000; ic += 1000) {
    B2INFO("DB constants for cellID=" << ic << ": ExpMuMuE = " << ExpMuMuE[ic - 1] << " ElectronicsCalib = " << ElectronicsCalib[ic - 1]
           << " MuMuECalib = " << MuMuECalib[ic - 1]);
  }

  /** Verify that we have valid values for the starting calibrations */
  for (int crysID = 0; crysID < 8736; crysID++) {
    if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclMuMuECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
    if (ExpMuMuE[crysID] == 0) {B2FATAL("eclMuMuECollector: ExpMuMuE = 0 for crysID = " << crysID);}
    if (MuMuECalib[crysID] == 0) {B2FATAL("eclMuMuECollector: MuMuECalib = 0 for crysID = " << crysID);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Required data objects */
  m_trackArray.isRequired();
  m_eclDigitArray.isRequired();

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclMuMuECollectorModule::collect()
{

  /** Record the input database constants for the first call */
  if (iEvent == 0) {
    for (int crysID = 0; crysID < 8736; crysID++) {
      getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysID + 0.001, ExpMuMuE[crysID]);
      getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
      getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysID + 0.001, MuMuECalib[crysID]);
      getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysID + 0.001);
    }
  }

  if (iEvent % 10000 == 0) {B2INFO("eclMuMuECollector: iEvent = " << iEvent);}
  iEvent++;

  /**----------------------------------------------------------------------------------------*/
  /** Check if DB objects have changed */
  if (m_ECLExpMuMuE.hasChanged()) { B2FATAL("eclMuMuECollector: ExpMuMuE has changed");}
  if (m_ElectronicsCalib.hasChanged()) {B2FATAL("eclMuMuECollector: ElectronicsCalib has changed");}
  if (m_MuMuECalib.hasChanged()) {
    B2INFO("eclMuMuECollector: new values for MuMuECalib");
    MuMuECalib = m_MuMuECalib->getCalibVector();
    for (int ic = 1; ic < 9000; ic += 1000) {
      B2INFO("Updated MuMuECalib for cellID=" << ic << ": MuMuECalib = " << MuMuECalib[ic - 1]);
    }
  }

  /**----------------------------------------------------------------------------------------*/
  /** If requested, require a level 1 trigger  */
  if (m_requireL1) {
    unsigned int L1TriggerResults = m_TRGResults->getTRGSummary(0);
    if (L1TriggerResults == 0) {return;}
  }

  //------------------------------------------------------------------------
  /** Event selection. First, require at least two tracks */
  int nTrack = m_trackArray.getEntries();
  if (nTrack < 2) {return;}

  /** Look for highest pt negative and positive tracks in specified theta lab region. Negative first, positive 2nd. Use the pion (211) mass hypothesis, only one that is always available */
  double maxpt[2] = {0., 0.};
  int iTrack[2] = { -1, -1};
  for (int it = 0; it < nTrack; it++) {
    const TrackFitResult* temptrackFit = m_trackArray[it]->getTrackFitResult(Const::ChargedStable(211));
    if (not temptrackFit) {continue;}
    int imu = 0;
    if (temptrackFit->getChargeSign() == 1) {imu = 1; }

    double temppt = temptrackFit->getTransverseMomentum();
    double cotThetaLab = temptrackFit->getCotTheta();
    if (temppt > maxpt[imu] && cotThetaLab > cotThetaLabMin && cotThetaLab < cotThetaLabMax) {
      maxpt[imu] = temppt;
      iTrack[imu] = it;
    }
  }

  /** Quit if we are missing a track */
  if (iTrack[0] == -1 || iTrack[1] == -1) { return; }

  /** Quit if the invariant mass of the two tracks is too low */
  TLorentzVector mu0 = m_trackArray[iTrack[0]]->getTrackFitResult(Const::ChargedStable(211))->get4Momentum();
  TLorentzVector mu1 = m_trackArray[iTrack[1]]->getTrackFitResult(Const::ChargedStable(211))->get4Momentum();
  if ((mu0 + mu1).M() < m_minPairMass) { return; }

  //------------------------------------------------------------------------
  /** Extrapolate these two tracks into the ECL using muon (13) hypothesis */
  int extCrysID[2] = { -1, -1};
  Const::EDetector eclID = Const::EDetector::ECL;
  for (int imu = 0; imu < 2; imu++) {
    TVector3 temppos[2] = {};
    int IDEnter = -99;
    for (auto& extHit : m_trackArray[iTrack[imu]]->getRelationsTo<ExtHit>()) {
      int pdgCode = extHit.getPdgCode();
      Const::EDetector detectorID = extHit.getDetectorID(); // subsystem ID
      int temp0 = extHit.getCopyID();  // ID within that subsystem; for ecl it is crystal ID

      /** This extrapolation is the entrance point to an ECL crystal, assuming muon hypothesis */
      if (detectorID == eclID && TMath::Abs(pdgCode) == 13 && extHit.getStatus() == EXT_ENTER) {
        IDEnter = temp0;
        temppos[0] = extHit.getPosition();
      }

      /** Now we have the exit point of the same ECL crystal */
      if (detectorID == eclID && TMath::Abs(pdgCode) == 13 && extHit.getStatus() == EXT_EXIT && temp0 == IDEnter) {
        temppos[1] = extHit.getPosition();

        /** Keep track of this crystal if the track length is long enough. Note that if minTrackLength is less than half the crystal length, we will keep only the first extrapolation due to break */
        double trackLength = (temppos[1] - temppos[0]).Mag();
        if (trackLength > m_minTrackLength) {extCrysID[imu] = temp0;}
        break;
      }
    }
  }

  /** Quit if neither track has a successful extrapolation */
  if (extCrysID[0] == -1 && extCrysID[1] == -1) { return; }

  //------------------------------------------------------------------------
  /** Record ECL digit amplitude as a function of CrysID */
  memset(&EperCrys[0], 0, EperCrys.size()*sizeof EperCrys[0]);
  for (auto& eclDigit : m_eclDigitArray) {
    int crysID = eclDigit.getCellId() - 1;
    getObjectPtr<TH2F>("RawDigitAmpvsCrys")->Fill(crysID + 0.001, eclDigit.getAmp());

    /** MuMuECalib is negative if the previous iteration of the algorithm was unable to calculate a value. In this case, the input value has been stored with a minus sign */
    EperCrys[crysID] = eclDigit.getAmp() * abs(MuMuECalib[crysID]) * ElectronicsCalib[crysID];
    if (EperCrys[crysID] > 0.01) {
      getObjectPtr<TH2F>("RawDigitTimevsCrys")->Fill(crysID + 0.001, eclDigit.getTimeFit());
    }
  }

  /** Overwrite using ECLCalDigits if we are using these events to determine MC deposited energy */
  if (m_measureTrueEnergy) {

    for (auto& eclCalDigit : m_eclCalDigitArray) {
      int tempCrysID = eclCalDigit.getCellId() - 1;
      EperCrys[tempCrysID] = eclCalDigit.getEnergy();
    }
  }

  //------------------------------------------------------------------------
  /** Require that the energy in immediately adjacent crystals is below threshold */
  for (int imu = 0; imu < 2; imu++) {
    int crysID = extCrysID[imu];
    int cellID = crysID + 1;
    if (crysID > -1) {

      bool noNeighbourSignal = true;
      if (cellID >= firstcellIDN4 && crysID <= lastcellIDN4) {
        for (const auto& tempCellID : myNeighbours4->getNeighbours(cellID)) {
          int tempCrysID = tempCellID - 1;
          if (tempCellID != cellID && EperCrys[tempCrysID] > m_MaxNeighbourE) {
            noNeighbourSignal = false;
            break;
          }
        }
      } else {
        for (const auto& tempCellID : myNeighbours8->getNeighbours(cellID)) {
          int tempCrysID = tempCellID - 1;
          if (tempCellID != cellID && EperCrys[tempCrysID] > m_MaxNeighbourE) {
            noNeighbourSignal = false;
            break;
          }
        }
      }

      /** Fill the histogram if no significant signal in a neighbouring crystal */
      if (noNeighbourSignal) {

        /** ExpMuMuE is negative if the algorithm was unable to calculate a value. In this case, the nominal input value has been stored with a minus sign */
        getObjectPtr<TH2F>("EnVsCrysID")->Fill(crysID + 0.001, EperCrys[crysID] / abs(ExpMuMuE[crysID]));
        getObjectPtr<TH1F>("ExpEvsCrys")->Fill(crysID + 0.001, ExpMuMuE[crysID]);
        getObjectPtr<TH1F>("ElecCalibvsCrys")->Fill(crysID + 0.001, ElectronicsCalib[crysID]);
        getObjectPtr<TH1F>("InitialCalibvsCrys")->Fill(crysID + 0.001, MuMuECalib[crysID]);
        getObjectPtr<TH1F>("CalibEntriesvsCrys")->Fill(crysID + 0.001);
      }
    }
  }
}
