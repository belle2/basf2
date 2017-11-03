/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclMuMuECollector/eclMuMuECollectorModule.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/RelationVector.h>
#include <tracking/dataobjects/ExtHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/gearbox/Const.h>


#include <TH2F.h>

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

//** we should have a parameter related to L1 and HLT trigger bits. For now, restrict the
//   tracks to the specified angular region covered by the L1 trigger
eclMuMuECollectorModule::eclMuMuECollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration Collector Module for ECL single crystal energy calibration using muons");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("minPairMass", m_minPairMass, "minimum invariant mass of the muon pair (GeV/c^2)", 9.0);
  addParam("minTrackLength", m_minTrackLength, "minimum extrapolated track length in the crystal (cm)", 30.);
  addParam("MaxNeighborAmp", m_MaxNeighborAmp, "maximum signal allowed in a neighboring crystal (adc counts)", 200.);
  addParam("thetaLabMinDeg", m_thetaLabMinDeg, "miniumum muon theta in lab (degrees)", 24.);
  addParam("thetaLabMaxDeg", m_thetaLabMaxDeg, "maximum muon theta in lab (degrees)", 134.);
  addParam("useTrueEnergy", m_useTrueEnergy, "store MC true deposited energy", false);
}

//-----------------------------------------------------------------------------------------------------
void eclMuMuECollectorModule::prepare()
{

  //..Create the histograms and register them in the data store
  auto EmuVsCellID0 = new TH2F("EmuVsCellID0", "ECLDigit amplitude vs cell ID;cellID from 0;Amplitude", 8736, 0, 8736, 240, 2000,
                               8000);
  auto MuonLabPvsCellID0 = new TH1F("MuonLabPvsCellID0", "Muon lab momentum for each cell;cellID from 0;Momentum (GeV/c)", 8736, 0,
                                    8736);
  registerObject<TH2F>("EmuVsCellID0", EmuVsCellID0);
  registerObject<TH1F>("MuonLabPvsCellID0", MuonLabPvsCellID0);

  //------------------------------------------------------------------------
  //..Four or ~eight nearest neighbors, plus crystal itself. cellID starts from 1 in ECLNeighbours
  myNeighbours4 = new ECLNeighbours("NC", 1);
  myNeighbours8 = new ECLNeighbours("N", 1);

  //------------------------------------------------------------------------
  //..Find the nominal (beam-energy) muon momentum for each crystal in the lab frame
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();
  PCmsLabTransform boostrotate;
  double beamE = 0.5 * boostrotate.getCMSEnergy();
  double muonP = sqrt(beamE * beamE - Const::muonMass * Const::muonMass);
  MuPlab.resize(8736);

  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    TVector3 CellPosition = eclp->GetCrystalPos(iECLCell);
    TLorentzVector CellLab(1., 1., 1., 1.);
    CellLab.SetTheta(CellPosition.Theta());
    CellLab.SetPhi(CellPosition.Phi());
    CellLab.SetRho(1.);
    CellLab.SetE(1.);
    TLorentzVector CellCOM = boostrotate.rotateLabToCms() * CellLab;

    //..Set mass and energy to match a beam energy muon
    CellCOM.SetE(beamE);
    CellCOM.SetRho(muonP);

    //..And boost back to lab frame
    TLorentzVector MuonLab = boostrotate.rotateCmsToLab() * CellCOM;
    MuPlab[iECLCell] = MuonLab.Rho();

    if (iECLCell % 1000 == 0) {
      B2DEBUG(1, "ECLGeom: " << iECLCell << " " << CellLab.Theta() << " " << CellLab.Phi() << " " << CellLab.M() << " " << CellLab.E());
      B2DEBUG(1, "     " << CellCOM.Theta() << " " << CellCOM.Phi() << " " << CellCOM.M() << " " << CellCOM.E());
      B2DEBUG(1, "     " << MuonLab.Theta() << " " << MuonLab.Phi() << " " << MuonLab.M() << " " << MuonLab.E() << " "  << MuonLab.Rho());
    }
  }


  //------------------------------------------------------------------------
  //..Parameters
  B2INFO("Input parameters to eclMuMuECollector:");
  B2INFO("minPairMass: " << m_minPairMass);
  B2INFO("minTrackLength: " << m_minTrackLength);
  B2INFO("MaxNeighborAmp: " << m_MaxNeighborAmp);
  B2INFO("thetaLabMinDeg: " << m_thetaLabMinDeg);
  B2INFO("thetaLabMaxDeg: " << m_thetaLabMaxDeg);
  B2INFO("useTrueEnergy: " << m_useTrueEnergy);
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

  //..Resize vectors
  EnergyPerCell.resize(8736);
}


//-----------------------------------------------------------------------------------------------------
void eclMuMuECollectorModule::collect()
{

  //..First event, record the muon kinematics
  if (iEvent == 0) {
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      getObjectPtr<TH1F>("MuonLabPvsCellID0")->SetBinContent(iECLCell + 1, MuPlab[iECLCell]);
      getObjectPtr<TH1F>("MuonLabPvsCellID0")->SetBinError(iECLCell + 1, 0);
    }
  }
  if (iEvent % 10000 == 0) {B2INFO("eclMuMuECollector: iEvent = " << iEvent);}
  iEvent++;

  //------------------------------------------------------------------------
  //..Event selection. First, require at least two tracks
  StoreArray<Track> TrackArray;
  int nTrack = TrackArray.getEntries();
  if (nTrack < 2) {return;}

  //..Look for highest pt negative and positive tracks in specified theta lab region. Negative first, positive 2nd
  double maxpt[2] = {0., 0.};
  int iTrack[2] = { -1, -1};
  for (int it = 0; it < nTrack; it++) {
    const TrackFitResult* temptrackFit = TrackArray[it]->getTrackFitResult(Const::ChargedStable(pdgmuon));
    if (not temptrackFit) {
      B2WARNING("Skipping track without myon hypothesis.");
      continue;
    }
    int imu = 0;
    if (temptrackFit->getChargeSign() == 1) {imu = 1; }
    double temppt = temptrackFit->getTransverseMomentum();
    double cotThetaLab = temptrackFit->getCotTheta();
    if (temppt > maxpt[imu] && cotThetaLab > cotThetaLabMin && cotThetaLab < cotThetaLabMax) {
      maxpt[imu] = temppt;
      iTrack[imu] = it;
    }
  }

  //..Quit if we are missing a track
  if (iTrack[0] == -1 || iTrack[1] == -1) { return; }

  //..Quit if the invariant mass of the two tracks is too low
  TLorentzVector mu0 = TrackArray[iTrack[0]]->getTrackFitResult(Const::ChargedStable(pdgmuon))->get4Momentum();
  TLorentzVector mu1 = TrackArray[iTrack[1]]->getTrackFitResult(Const::ChargedStable(pdgmuon))->get4Momentum();
  if ((mu0 + mu1).M() < m_minPairMass) { return; }

  //------------------------------------------------------------------------
  //..Extrapolate these two tracks into the ECL
  int extCellID0[2] = { -1, -1};
  Const::EDetector eclID = Const::EDetector::ECL;
  for (int imu = 0; imu < 2; imu++) {
    TVector3 temppos[2] = {};
    int IDEnter = -99;
    for (auto& extHit : TrackArray[iTrack[imu]]->getRelationsTo<ExtHit>()) {
      int pdgCode = extHit.getPdgCode();
      Const::EDetector detectorID = extHit.getDetectorID(); // subsystem ID
      int tempID0 = extHit.getCopyID();  // ID within that subsystem; for ecl it is cellID from 0

      //..This extrapolation is the entrance point to an ECL crystal, assuming muon hypothesis
      if (detectorID == eclID && TMath::Abs(pdgCode) == pdgmuon && extHit.getStatus() == EXT_ENTER) {
        IDEnter = tempID0;
        temppos[0] = extHit.getPosition();
      }

      //..Now we have the exit point of the same ECL crystal
      if (detectorID == eclID && TMath::Abs(pdgCode) == pdgmuon && extHit.getStatus() == EXT_EXIT && tempID0 == IDEnter) {
        temppos[1] = extHit.getPosition();

        //..Keep track of this crystal if the track length is long enough. Note that if minTrackLength
        //  is less than half the crystal length, we will keep only the first extrapolation due to break
        double trackLength = (temppos[1] - temppos[0]).Mag();
        if (trackLength > m_minTrackLength) {extCellID0[imu] = tempID0;}
        break;
      }
    }
  }

  //..Quit if neither track has a successful extrapolation
  if (extCellID0[0] == -1 && extCellID0[1] == -1) { return; }

  //------------------------------------------------------------------------
  //..MC true deposited energy, if requested
  double TrueEnergy[2] = {};
  if (m_useTrueEnergy) {
    StoreArray<ECLCalDigit> eclCalDigitArray;
    for (auto& eclCalDigit : eclCalDigitArray) {
      int tempcellID0 = eclCalDigit.getCellId() - 1;
      int imu = -1;
      if (tempcellID0 == extCellID0[0]) {imu = 0;}
      if (tempcellID0 == extCellID0[1]) {imu = 1;}
      if (imu >= 0) {
        auto relatedParticlePairs = eclCalDigit.getRelationsWith<MCParticle>();
        int nrel = (int)relatedParticlePairs.size();
        for (int irel = 0; irel < nrel; irel++) { TrueEnergy[imu] += relatedParticlePairs.weight(irel);}
      }
    }
  }

  //------------------------------------------------------------------------
  //..Record ECL digit amplitude as a function of cellID0
  memset(&EnergyPerCell[0], 0, EnergyPerCell.size()*sizeof EnergyPerCell[0]);
  StoreArray<ECLDigit> eclDigitArray;
  for (auto& eclDigit : eclDigitArray) {
    int tempID0 = eclDigit.getCellId() - 1;
    EnergyPerCell[tempID0] = eclDigit.getAmp();
  }

  //..Require that the energy in immediately adjacent crystals is below threshold
  for (int imu = 0; imu < 2; imu++) {
    int ID0 = extCellID0[imu];
    if (ID0 > -1) {

      bool noNeighborSignal = true;
      if (ID0 >= firstID0N4 && ID0 <= lastID0N4) {
        for (const auto& tempID1 : myNeighbours4->getNeighbours(ID0 + 1)) {
          if (tempID1 - 1 != ID0 && EnergyPerCell[tempID1 - 1] > m_MaxNeighborAmp) {noNeighborSignal = false; break;}
        }
      } else {
        for (const auto& tempID1 : myNeighbours8->getNeighbours(ID0 + 1)) {
          if (tempID1 - 1 != ID0 && EnergyPerCell[tempID1 - 1] > m_MaxNeighborAmp) {noNeighborSignal = false; break;}
        }
      }

      //..Fill the histogram if no significant signal in a neighboring crystal
      if (noNeighborSignal) {
        double eStore = EnergyPerCell[extCellID0[imu]];
        if (m_useTrueEnergy) {eStore = MCCalibConstant * TrueEnergy[imu];}
        getObjectPtr<TH2F>("EmuVsCellID0")->Fill(extCellID0[imu] + 0.001, eStore);
      }
    }
  }
}
