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
#include <ecl/dataobjects/ECLDigit.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/RelationVector.h>
#include <tracking/dataobjects/ExtHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/utility/PCmsLabTransform.h>


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
  addParam("minPairMass", m_minPairMass, "minimum invariant mass of the muon pair", 9.0);
  addParam("minTrackLength", m_minTrackLength, "minimum extrapolated track length in the crystal", 30.);
  addParam("MaxNeighborAmp", m_MaxNeighborAmp, "maximum signal allowed in a neighboring crystal", 200.);
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
  //..ECL geometry. Need to find all immediate neighbors of each crystal. Does not include crystal itself.
  //  Crystals on the corners are neighbors only in the endcaps
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();
  const int nThetaRings = 69; // number of theta rings in the ECL
  const short nCrystalsPerRing[nThetaRings] = {
    48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, // Forward 0--12
    144, 144, 144, 144, 144, 144, 144,  // Barrel 13--58
    144, 144, 144, 144, 144, 144, 144, 144, 144, 144,
    144, 144, 144, 144, 144, 144, 144, 144, 144, 144,
    144, 144, 144, 144, 144, 144, 144, 144, 144, 144,
    144, 144, 144, 144, 144, 144, 144, 144, 144,
    144, 144, 96, 96, 96, 96, 96, 64, 64, 64 // Backward 59--68
  };

  for (int it = 0; it < nThetaRings; it++) {
    for (int ip = 0; ip < nCrystalsPerRing[it]; ip++) {

      //..cellID from 0, position, and phi of this crystal
      int ID0 = eclp->GetCellID(it, ip);
      TVector3 position = eclp->GetCrystalPos(ID0);
      float phi0 = position.Phi();
      int nc = 0; // number of neighbors for this crystal

      //..Theta ring range containing the neighbors
      int itlow = it - 1;
      if (itlow < 0) { itlow = 0;}
      int ithigh = it + 1;
      if (ithigh > 68) {ithigh = 68;}
      for (int nt = itlow; nt <= ithigh; nt++) {

        //..Neighbors in the same theta ring just have phiID +/- 1
        if (nt == it) {
          for (int dp = -1; dp < 2; dp += 2) {
            int np = ip + dp;
            if (np < 0) {np += nCrystalsPerRing[nt];}
            if (np >= nCrystalsPerRing[nt]) {np = np - nCrystalsPerRing[nt];}
            ListOfNeighbors[ID0][nc] = eclp->GetCellID(nt, np);
            nc++;
          }

          //..Adjacent theta ring in the region where all rings have 144 crystals. Neighbor has
          //  the same phi ID as the central crystal
        } else if (it >= 12 && it <= 59) {
          ListOfNeighbors[ID0][nc] = eclp->GetCellID(nt, ip);
          nc++;

          //..Adjacent theta ring in the encaps. Closest crystal in phi, plus one on either side
        } else {
          int npc = ip;
          float dphiMin = 99.;
          for (int kp = 0; kp < nCrystalsPerRing[nt]; kp++) {
            int IDk = eclp->GetCellID(nt, kp);
            TVector3 temppos =  eclp->GetCrystalPos(IDk);
            float phik = temppos.Phi();
            float dphi = abs(phi0 - phik);
            if (dphi > TMath::Pi()) {dphi = 2 * TMath::Pi() - dphi; }
            if (dphi < dphiMin) {
              dphiMin = dphi;
              npc = kp;
            }
          }

          //..found the closest crystal in phi, now add neighbors to the list
          for (int dp = -1; dp < 2; dp++) {
            int np = npc + dp;
            if (np < 0) {np += nCrystalsPerRing[nt];}
            if (np >= nCrystalsPerRing[nt]) {np = np - nCrystalsPerRing[nt];}
            ListOfNeighbors[ID0][nc] = eclp->GetCellID(nt, np);
            nc++;
          }
        }
      }
      nNeighbors[ID0] = nc;
    }
  }

  //------------------------------------------------------------------------
  //..Find the nominal (beam-energy) muon momentum for each crystal in the lab frame
  PCmsLabTransform boostrotate;
  double beamE = 0.5 * boostrotate.getCMSEnergy();
  double muonP = sqrt(beamE * beamE - 0.0111637); // number is muon mass squared

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

    int itemp = iECLCell / 1000;
    if (iECLCell == 1000 * itemp) {
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
  double thetaLabMin = m_thetaLabMinDeg * TMath::Pi() / 180.;
  cotThetaLabMax = 1. / tan(thetaLabMin);
  double thetaLabMax = m_thetaLabMaxDeg * TMath::Pi() / 180.;
  cotThetaLabMin = 1. / tan(thetaLabMax);

}

//-----------------------------------------------------------------------------------------------------
void eclMuMuECollectorModule::collect()
{

  //..First event, record the muon kinematics
  if (iEvent == 0) {
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      getObject<TH1F>("MuonLabPvsCellID0").SetBinContent(iECLCell + 1, MuPlab[iECLCell]);
      getObject<TH1F>("MuonLabPvsCellID0").SetBinError(iECLCell + 1, 0);
    }
  }
  iEvent++;
  int n10000 = iEvent / 10000;
  if (10000 * n10000 == iEvent) {B2INFO("eclMuMuECollector: iEvent = " << iEvent);}

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
  //..Store ECL digit amplitude as a function of cellID0
  float EnergyPerCell[8736] = {};
  StoreArray<ECLDigit> eclDigitArray;
  for (auto& eclDigit : eclDigitArray) {
    int tempID0 = eclDigit.getCellId() - 1;
    EnergyPerCell[tempID0] = eclDigit.getAmp();
  }

  //..Require that the energy in immediately adjacent crystals is below threshold
  for (int imu = 0; imu < 2; imu++) {
    if (extCellID0[imu] > -1) {

      bool noNeighborSignal = true;
      for (int in = 0; in < nNeighbors[extCellID0[imu]]; in++) {
        int tempID0 = ListOfNeighbors[extCellID0[imu]][in];
        float tempAmp = EnergyPerCell[tempID0];
        if (tempAmp > m_MaxNeighborAmp) {noNeighborSignal = false; break;}
      }

      //..Fill the histogram if no significant signal in a neighboring crystal
      if (noNeighborSignal) {
        double eStore = EnergyPerCell[extCellID0[imu]];
        if (m_useTrueEnergy) {eStore = MCCalibConstant * TrueEnergy[imu];}
        getObject<TH2F>("EmuVsCellID0").Fill(extCellID0[imu] + 0.001, eStore);
      }
    }
  }
}
