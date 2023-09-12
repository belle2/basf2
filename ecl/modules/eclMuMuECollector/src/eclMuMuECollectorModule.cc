/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclMuMuECollector/eclMuMuECollectorModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/geometry/ECLNeighbours.h>

/* Basf2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TH2F.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace ECL;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclMuMuECollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

eclMuMuECollectorModule::eclMuMuECollectorModule() : CalibrationCollectorModule(), m_ECLExpMuMuE("ECLExpMuMuE"),
  m_ElectronicsCalib("ECLCrystalElectronics"), m_MuMuECalib("ECLCrystalEnergyMuMu"), m_CrystalEnergy("ECLCrystalEnergy")
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
  auto TrkPerCrysID = new TH1F("TrkPerCrysID", "track extrapolations per crystalID;crystal ID", ECLElementNumbers::c_NCrystals, 0,
                               ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("TrkPerCrysID", TrkPerCrysID);

  auto EnVsCrysID = new TH2F("EnVsCrysID", "Normalized energy for each crystal;crystal ID;E/Expected", ECLElementNumbers::c_NCrystals,
                             0, ECLElementNumbers::c_NCrystals, 240, 0.1, 2.5);
  registerObject<TH2F>("EnVsCrysID", EnVsCrysID);

  auto ExpEvsCrys = new TH1F("ExpEvsCrys", "Sum expected energy vs crystal ID;crystal ID;Energy (GeV)",
                             ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("ExpEvsCrys", ExpEvsCrys);

  auto ElecCalibvsCrys = new TH1F("ElecCalibvsCrys", "Sum electronics calib const vs crystal ID;crystal ID;calibration constant",
                                  ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("ElecCalibvsCrys", ElecCalibvsCrys);

  auto InitialCalibvsCrys = new TH1F("InitialCalibvsCrys",
                                     "Sum initial muon pair calib const vs crystal ID;crystal ID;calibration constant", ECLElementNumbers::c_NCrystals, 0,
                                     ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("InitialCalibvsCrys", InitialCalibvsCrys);

  auto CalibEntriesvsCrys = new TH1F("CalibEntriesvsCrys", "Entries in calib vs crys histograms;crystal ID;Entries per crystal",
                                     ECLElementNumbers::c_NCrystals,
                                     0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1F>("CalibEntriesvsCrys", CalibEntriesvsCrys);

  /** Raw digit quantities for debugging purposes only */
  auto RawDigitAmpvsCrys = new TH2F("RawDigitAmpvsCrys", "Digit Amplitude vs crystal ID;crystal ID;Amplitude",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals, 250, 0,
                                    25000);
  registerObject<TH2F>("RawDigitAmpvsCrys", RawDigitAmpvsCrys);

  auto RawDigitTimevsCrys = new TH2F("RawDigitTimevsCrys", "Digit Time vs crystal ID;crystal ID;Time", ECLElementNumbers::c_NCrystals,
                                     0, ECLElementNumbers::c_NCrystals, 200, -2000,
                                     2000);
  registerObject<TH2F>("RawDigitTimevsCrys", RawDigitTimevsCrys);

  //..Diagnose possible cable swaps
  auto hitCrysVsExtrapolatedCrys = new TH2F("hitCrysVsExtrapolatedCrys",
                                            "Crystals with high energy vs extrapolated crystals with no energy;extrapolated crystalID;High crystalID",
                                            ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals, ECLElementNumbers::c_NCrystals, 0,
                                            ECLElementNumbers::c_NCrystals);
  registerObject<TH2F>("hitCrysVsExtrapolatedCrys", hitCrysVsExtrapolatedCrys);


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
    double thetaLabMin = m_thetaLabMinDeg * TMath::DegToRad();
    cotThetaLabMax = 1. / tan(thetaLabMin);
  }
  if (m_thetaLabMaxDeg < 1.) {
    cotThetaLabMin = 9999.;
  } else if (m_thetaLabMaxDeg > 179.) {
    cotThetaLabMin = -9999.;
  } else {
    double thetaLabMax = m_thetaLabMaxDeg * TMath::DegToRad();
    cotThetaLabMin = 1. / tan(thetaLabMax);
  }
  B2INFO("cotThetaLabMin: " << cotThetaLabMin);
  B2INFO("cotThetaLabMax: " << cotThetaLabMax);
  B2INFO("measureTrueEnergy: " << m_measureTrueEnergy);
  B2INFO("requireL1: " << m_requireL1);

  /** Resize vectors */
  EperCrys.resize(ECLElementNumbers::c_NCrystals);

  /**----------------------------------------------------------------------------------------*/
  /** Get expected energies and calibration constants from DB. Need to call hasChanged() for later comparison */
  if (m_ECLExpMuMuE.hasChanged()) {ExpMuMuE = m_ECLExpMuMuE->getCalibVector();}
  if (m_ElectronicsCalib.hasChanged()) {ElectronicsCalib = m_ElectronicsCalib->getCalibVector();}
  if (m_MuMuECalib.hasChanged()) {MuMuECalib = m_MuMuECalib->getCalibVector();}
  if (m_CrystalEnergy.hasChanged()) {CrystalEnergy = m_CrystalEnergy->getCalibVector();}

  /** Write out a few for quality control */
  for (int ic = 1; ic < 9000; ic += 1000) {
    B2INFO("DB constants for cellID=" << ic << ": ExpMuMuE = " << ExpMuMuE[ic - 1] << " ElectronicsCalib = " << ElectronicsCalib[ic - 1]
           << " MuMuECalib = " << MuMuECalib[ic - 1]);
  }

  /** Verify that we have valid values for the starting calibrations */
  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
    if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclMuMuECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
    if (ExpMuMuE[crysID] == 0) {B2FATAL("eclMuMuECollector: ExpMuMuE = 0 for crysID = " << crysID);}
    if (MuMuECalib[crysID] == 0) {B2FATAL("eclMuMuECollector: MuMuECalib = 0 for crysID = " << crysID);}
  }

  /**----------------------------------------------------------------------------------------*/
  /** Required data objects */
  m_trackArray.isRequired();
  m_eclDigitArray.isRequired();
  if (m_measureTrueEnergy) {m_eclClusterArray.isRequired();}

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclMuMuECollectorModule::collect()
{

  /** Record the input database constants for the first call */
  if (iEvent == 0) {
    for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
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
  bool newConst = false;
  if (m_ECLExpMuMuE.hasChanged()) {
    newConst = true;
    B2INFO("ECLExpMuMuE has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    ExpMuMuE = m_ECLExpMuMuE->getCalibVector();
  }
  if (m_ElectronicsCalib.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalElectronics has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    ElectronicsCalib = m_ElectronicsCalib->getCalibVector();
  }
  if (m_MuMuECalib.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalEnergyMuMu has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    MuMuECalib = m_MuMuECalib->getCalibVector();
  }
  if (m_CrystalEnergy.hasChanged()) {
    newConst = true;
    B2INFO("ECLCrystalEnergy has changed, exp = " << m_evtMetaData->getExperiment() << "  run = " << m_evtMetaData->getRun());
    CrystalEnergy = m_CrystalEnergy->getCalibVector();
  }

  if (newConst) {
    for (int ic = 1; ic < 9000; ic += 1000) {
      B2INFO("DB constants for cellID=" << ic << ": ExpMuMuE = " << ExpMuMuE[ic - 1] << " ElectronicsCalib = " <<
             ElectronicsCalib[ic - 1]
             << " MuMuECalib = " << MuMuECalib[ic - 1]);
    }

    /** Verify that we have valid values for the starting calibrations */
    for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
      if (ElectronicsCalib[crysID] <= 0) {B2FATAL("eclMuMuECollector: ElectronicsCalib = " << ElectronicsCalib[crysID] << " for crysID = " << crysID);}
      if (ExpMuMuE[crysID] == 0) {B2FATAL("eclMuMuECollector: ExpMuMuE = 0 for crysID = " << crysID);}
      if (MuMuECalib[crysID] == 0) {B2FATAL("eclMuMuECollector: MuMuECalib = 0 for crysID = " << crysID);}
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
  ROOT::Math::PxPyPzEVector mu0 = m_trackArray[iTrack[0]]->getTrackFitResult(Const::ChargedStable(211))->get4Momentum();
  ROOT::Math::PxPyPzEVector mu1 = m_trackArray[iTrack[1]]->getTrackFitResult(Const::ChargedStable(211))->get4Momentum();
  if ((mu0 + mu1).M() < m_minPairMass) { return; }

  //------------------------------------------------------------------------
  /** Extrapolate these two tracks into the ECL using muon (13) hypothesis */
  int extCrysID[2] = { -1, -1};
  Const::EDetector eclID = Const::EDetector::ECL;
  for (int imu = 0; imu < 2; imu++) {
    ROOT::Math::XYZVector temppos[2] = {};
    int IDEnter = -99;
    for (auto& extHit : m_trackArray[iTrack[imu]]->getRelationsTo<ExtHit>()) {
      int pdgCode = extHit.getPdgCode();
      Const::EDetector detectorID = extHit.getDetectorID(); // subsystem ID
      int temp0 = extHit.getCopyID();  // ID within that subsystem; for ecl it is crystal ID

      /** This extrapolation is the entrance point to an ECL crystal, assuming muon hypothesis */
      if (detectorID == eclID && TMath::Abs(pdgCode) == Const::muon.getPDGCode() && extHit.getStatus() == EXT_ENTER) {
        IDEnter = temp0;
        temppos[0] = extHit.getPosition();
      }

      /** Now we have the exit point of the same ECL crystal */
      if (detectorID == eclID && TMath::Abs(pdgCode) == Const::muon.getPDGCode() && extHit.getStatus() == EXT_EXIT && temp0 == IDEnter) {
        temppos[1] = extHit.getPosition();

        /** Keep track of this crystal if the track length is long enough. Note that if minTrackLength is less than half the crystal length, we will keep only the first extrapolation due to break */
        double trackLength = (temppos[1] - temppos[0]).R();
        if (trackLength > m_minTrackLength) {extCrysID[imu] = temp0;}
        break;
      }
    }
  }

  /** Quit if neither track has a successful extrapolation */
  if (extCrysID[0] == -1 && extCrysID[1] == -1) { return; }

  //------------------------------------------------------------------------
  /** Record ECL energy for each crystal */
  std::fill(EperCrys.begin(), EperCrys.end(), 0); // clear array

  //..Record crystals with high energies to diagnose cable swaps
  const double highEnergyThresh = 0.18; // GeV
  std::vector<int> highECrys; // crystalIDs of crystals with high energy

  //..For data, use muon pair calibration; for expected energies, use ECLCrystalEnergy
  for (auto& eclDigit : m_eclDigitArray) {
    int crysID = eclDigit.getCellId() - 1;
    getObjectPtr<TH2F>("RawDigitAmpvsCrys")->Fill(crysID + 0.001, eclDigit.getAmp());

    /** MuMuECalib is negative if the previous iteration of the algorithm was unable to calculate a value. In this case, the input value has been stored with a minus sign */
    float calib =  abs(MuMuECalib[crysID]);
    if (m_measureTrueEnergy) {calib = CrystalEnergy[crysID];}
    EperCrys[crysID] = eclDigit.getAmp() * calib * ElectronicsCalib[crysID];
    if (EperCrys[crysID] > highEnergyThresh) {highECrys.push_back(crysID);}
    if (EperCrys[crysID] > 0.01) {
      getObjectPtr<TH2F>("RawDigitTimevsCrys")->Fill(crysID + 0.001, eclDigit.getTimeFit());
    }
  }

  //------------------------------------------------------------------------
  //..For expected energies, get the max energies crystals from the cluster. This is
  // safer than converting the ECLDigit, since it does not require that the ECLCrystalEnergy
  // payload used now is the same as was used when the event was generated.
  if (m_measureTrueEnergy) {
    for (int ic = 0; ic < m_eclClusterArray.getEntries(); ic++) {
      if (m_eclClusterArray[ic]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        int crysID = m_eclClusterArray[ic]->getMaxECellId() - 1;
        float undoCorrection = m_eclClusterArray[ic]->getEnergyRaw() / m_eclClusterArray[ic]->getEnergy(
                                 ECLCluster::EHypothesisBit::c_nPhotons);
        EperCrys[crysID] = undoCorrection * m_eclClusterArray[ic]->getEnergyHighestCrystal();
      }
    }
  }

  //------------------------------------------------------------------------
  /** Require that the energy in immediately adjacent crystals is below threshold.
      Also check if neighbour has high energy due to cable swaps */
  for (int imu = 0; imu < 2; imu++) {
    int crysID = extCrysID[imu];
    int cellID = crysID + 1;
    if (crysID > -1) {

      getObjectPtr<TH1F>("TrkPerCrysID")->Fill(crysID + 0.001);

      bool noNeighbourSignal = true;
      bool highNeighourSignal = false;
      if (cellID >= firstcellIDN4 && crysID <= lastcellIDN4) {
        for (const auto& tempCellID : myNeighbours4->getNeighbours(cellID)) {
          int tempCrysID = tempCellID - 1;
          if (tempCellID != cellID && EperCrys[tempCrysID] > m_MaxNeighbourE) {
            noNeighbourSignal = false;
            if (EperCrys[tempCrysID] > highEnergyThresh) {highNeighourSignal = true;}
          }
        }
      } else {
        for (const auto& tempCellID : myNeighbours8->getNeighbours(cellID)) {
          int tempCrysID = tempCellID - 1;
          if (tempCellID != cellID && EperCrys[tempCrysID] > m_MaxNeighbourE) {
            noNeighbourSignal = false;
            if (EperCrys[tempCrysID] > highEnergyThresh) {highNeighourSignal = true;}
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

      //..Possible cable swap
      if (highNeighourSignal or (noNeighbourSignal and EperCrys[crysID] < m_MaxNeighbourE)) {
        for (auto& id : highECrys) {
          getObjectPtr<TH2F>("hitCrysVsExtrapolatedCrys")->Fill(crysID + 0.0001, id + 0.0001);
        }
      }
    }
  }
}
