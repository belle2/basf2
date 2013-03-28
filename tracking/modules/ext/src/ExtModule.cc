/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/ext/ExtModule.h>
#include <tracking/modules/ext/ExtManager.h>
#include <tracking/modules/ext/ExtPhysicsList.h>
#include <tracking/modules/ext/ExtCylSurfaceTarget.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/MagneticField.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <TMatrixD.h>
#include <TVectorD.h>
#include <TVector3.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <GFTrack.h>
#include <GFDetPlane.h>
#include <GFFieldManager.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <globals.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4UImanager.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4RegionStore.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorPropagator.hh>
#include <G4ErrorTrackLengthTarget.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>

using namespace std;
using namespace Belle2;

REG_MODULE(Ext)

ExtModule::ExtModule() : Module(), m_extMgr(NULL)  // no ExtManager yet
{
  m_pdgCode.clear();
  setDescription("Extrapolates tracks from CDC to outer detectors using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("pdgCodes", m_pdgCode, "Positive-charge PDG codes for extrapolation hypotheses", m_pdgCode);
  addParam("TracksColName", m_TracksColName, "Name of collection holding the reconstructed tracks", string("Tracks"));
  addParam("ExtHitsColName", m_extHitsColName, "Name of collection holding the ExtHits from the extrapolation", string("ExtHits"));
  addParam("MinPt", m_minPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated.", double(0.0));
  addParam("MinKE", m_minKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation.", double(0.002));
  addParam("MaxStep", m_maxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity).", double(25.0));
  addParam("Cosmic", m_cosmic, "Particle source (0 = beam, 1 = cosmic ray.", 0);
}

ExtModule::~ExtModule()
{
}

void ExtModule::initialize()
{

  // Convert from GeV to GEANT4 energy units (MeV)
  m_minKE = m_minKE * GeV;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Define the geant4e extrapolation Manager.
  m_extMgr = ExtManager::GetManager();

  // See if ext will coexist with geant4 simulation.
  // (The particle list will have been constructed already, if so.)
  if (G4ParticleTable::GetParticleTable()->entries() == 0) {
    // ext will run without simulation
    m_runMgr = NULL;
    m_trk    = NULL;
    m_stp    = NULL;
    m_extMgr->SetUserInitialization(new DetectorConstruction());
    G4Region* region = (*(G4RegionStore::GetInstance()))[0];
    region->SetProductionCuts(G4ProductionCutsTable::GetProductionCutsTable()->GetDefaultProductionCuts());
    m_extMgr->SetUserInitialization(new ExtPhysicsList);
    //Create the magnetic field for the Geant4e simulation
    Simulation::MagneticField* magneticField = new Simulation::MagneticField();
    G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    fieldManager->SetDetectorField(magneticField);
    fieldManager->CreateChordFinder(magneticField);
    m_extMgr->InitGeant4e();
  } else {
    // ext will coexist with simulation
    m_runMgr = G4RunManager::GetRunManager();
    m_trk    = const_cast<G4UserTrackingAction*>(m_runMgr->GetUserTrackingAction());
    m_stp    = const_cast<G4UserSteppingAction*>(m_runMgr->GetUserSteppingAction());
    m_extMgr->InitGeant4e();
    G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
  }

  // Redefine ext's step length, magnetic field step limitation (fraction of local curvature radius),
  // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  G4double maxStep = std::min(10.0, m_maxStep) * cm;
  if (maxStep > 0.0) {
    char stepSize[80];
    std::sprintf(stepSize, "/geant4e/limits/stepLength %8.2f mm", maxStep);
    G4UImanager::GetUIpointer()->ApplyCommand(stepSize);
  }
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  GearDir strContent = GearDir("Detector/DetectorComponent[@name=\"COIL\"]/Content/");
  double offsetZ = strContent.getLength("OffsetZ") * cm;
  double rMin = strContent.getLength("Cryostat/Rmin") * cm;
  double halfLength = strContent.getLength("Cryostat/HalfLength") * cm;
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(new
                                                             ExtCylSurfaceTarget(rMin, offsetZ - halfLength, offsetZ + halfLength));

  // Hypotheses for extrapolation
  if (m_pdgCode.empty()) {
    m_chargedStable.push_back(Const::pion);
    m_chargedStable.push_back(Const::electron);
    m_chargedStable.push_back(Const::muon);
    m_chargedStable.push_back(Const::kaon);
    m_chargedStable.push_back(Const::proton);
  } else { // user defined
    std::vector<Const::ChargedStable> stack;
    stack.push_back(Const::pion);
    stack.push_back(Const::electron);
    stack.push_back(Const::muon);
    stack.push_back(Const::kaon);
    stack.push_back(Const::proton);
    for (unsigned i = 0; i < m_pdgCode.size(); ++i) {
      for (unsigned k = 0; k < stack.size(); ++k) {
        if (abs(m_pdgCode[i]) == stack[k].getPDGCode()) {
          m_chargedStable.push_back(stack[k]);
          stack.erase(stack.begin() + k);
          --k;
        }
      }
    }
    if (m_chargedStable.empty()) B2ERROR("Ext initialize(): no valid PDG codes for extrapolation")
    }

  for (unsigned i = 0; i < m_chargedStable.size(); ++i) {
    B2INFO("Module ext initialize(): hypothesis for PDG code "
           << m_chargedStable[i].getPDGCode() << " and its antiparticle will be extrapolated");
  }

  // Register output and relation arrays
  StoreArray<ExtHit>::registerPersistent();
  RelationArray::registerPersistent<Track, ExtHit>();

  return;

}

void ExtModule::beginRun()
{
}

void ExtModule::event()
{

  // Disable simulation-specific actions temporarily while we extrapolate
  if (m_runMgr) {
    m_runMgr->SetUserAction((G4UserTrackingAction*)NULL);
    m_runMgr->SetUserAction((G4UserSteppingAction*)NULL);
  }

  // Loop over the reconstructed tracks.
  // Do extrapolation for each hypotheses (pion, electron, muon, kaon, proton)
  // of each reconstructed track.
  // Pion hypothesis:  extrapolate until calorimeter exit
  // Other hypotheses: extrapolate up to but not including calorimeter

  StoreArray<Track> Tracks(m_TracksColName);
  StoreArray<ExtHit> extHits(m_extHitsColName);
  RelationArray TrackToExtHits(Tracks, extHits);

  G4ThreeVector position;
  G4ThreeVector momentum;
  G4ErrorTrajErr covG4e(5, 0);

  int nTracks = Tracks.getEntries();
  for (int t = 0; t < nTracks; ++t) {

    for (unsigned int hypothesis = 0; hypothesis < m_chargedStable.size(); hypothesis++) {

      Const::ChargedStable chargedStable = m_chargedStable[hypothesis];

      const TrackFitResult* trackFit = Tracks[t]->getTrackFitResult(chargedStable);
      if (!trackFit) {
        B2ERROR("Ext::event(): no valid TrackFitResult for PDGcode " <<
                chargedStable.getPDGCode() << ": extrapolation not possible")
        continue;
      }

      const GFTrack* gfTrack = DataStore::getRelated<GFTrack>(trackFit);
      if (!gfTrack) {
        B2ERROR("Ext::event(): no relation of TrackFitResult with GFTrack for PDGcode " <<
                chargedStable.getPDGCode() << ": extrapolation not possible")
        continue;
      }

      int charge = int(gfTrack->getCardinalRep()->getCharge());
      int pdgCode = chargedStable.getPDGCode() * charge;
      if (chargedStable == Const::electron || chargedStable == Const::muon) pdgCode = -pdgCode;

      getStartPoint(gfTrack, pdgCode, position, momentum, covG4e);
      if (gfTrack->getMom().Pt() <= m_minPt) continue;
      G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
      string g4eName = "g4e_" + particle->GetParticleName();
      double mass = particle->GetPDGMass();
      double minP = sqrt((mass + m_minKE) * (mass + m_minKE) - mass * mass);
      G4ErrorFreeTrajState* state = new G4ErrorFreeTrajState(g4eName, position, momentum, covG4e);
      m_extMgr->InitTrackPropagation();
      while (true) {
        const G4int    errCode    = m_extMgr->PropagateOneStep(state, G4ErrorMode_PropForwards);
        G4Track*       track      = state->GetG4Track();
        const G4Step*  step       = track->GetStep();
        const G4double length     = step->GetStepLength();
        const G4int    preStatus  = step->GetPreStepPoint()->GetStepStatus();
        const G4int    postStatus = step->GetPostStepPoint()->GetStepStatus();
        // First step on this track?
        if (preStatus == fUndefined) {
          createHit(state, EXT_FIRST, t, pdgCode, extHits, TrackToExtHits);
        }
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (length > 0.0) {
          if (preStatus == fGeomBoundary) {      // first step in this volume?
            createHit(state, EXT_ENTER, t, pdgCode, extHits, TrackToExtHits);
          }
          m_tof += step->GetDeltaTime();
          // Last step in this volume?
          if (postStatus == fGeomBoundary) {
            createHit(state, EXT_EXIT, t, pdgCode, extHits, TrackToExtHits);
          }
        }
        // Post-step momentum too low?
        if (errCode || (track->GetMomentum().mag() < minP)) {
          createHit(state, EXT_STOP, t, pdgCode, extHits, TrackToExtHits);
          break;
        }
        if (G4ErrorPropagatorData::GetErrorPropagatorData()->GetState() == G4ErrorState(G4ErrorState_TargetCloserThanBoundary)) {
          createHit(state, EXT_ESCAPE, t, pdgCode, extHits, TrackToExtHits);
          break;
        }
        if (m_extMgr->GetPropagator()->CheckIfLastStep(track)) {
          createHit(state, EXT_ESCAPE, t, pdgCode, extHits, TrackToExtHits);
          break;
        }

      } // track-extrapolation "infinite" loop

      m_extMgr->EventTermination();

      delete state;

    } // hypothesis loop

  } // track loop

  if (m_runMgr) {
    m_runMgr->SetUserAction(m_trk);
    m_runMgr->SetUserAction(m_stp);
  }

}

void ExtModule::endRun()
{
}

void ExtModule::terminate()
{

  if (m_runMgr) {
    m_runMgr->SetUserAction(m_trk);
    m_runMgr->SetUserAction(m_stp);
  }
  delete m_enter;
  delete m_exit;

}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void ExtModule::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("Module ext registerVolumes(): No geometry defined. Please create the geometry first.")
  }

  m_enter = new vector<G4VPhysicalVolume*>;
  m_exit  = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // TOP doesn't have one envelope; it has several "PlacedTOPModule"s
    if (name == "PlacedTOPModule") {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    // TOP quartz bar (=sensitive) has an automatically generated PV name
    // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
    // YYY is cuttest.
    if (name.find("_cuttest_") != string::npos) {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    if ((name == "ARICH.Envelope") || (name == "ARICH.DetectorModules")) {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    if (name == "moduleSensitive") {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    // ECL
    if (name == "physicalECL") {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    // ECL crystal (=sensitive) has an automatically generated PV name
    // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
    // YYY is logicalEcl**Crystal, where **=Br, Fw, or Bw.
    // XXX is 1..144 for Br, 1..16 for Fw, and 1..16 for Bw
    // ZZZ is n_pv_m where n is 1..46 for Br, 1..72 for Fw, and 73..132 for Bw
    // CopyNo() encodes XXX and n.
    if (name.find("_logicalEclBrCrystal_") != string::npos) {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    if (name.find("_logicalEclFwCrystal_") != string::npos) {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    if (name.find("_logicalEclBwCrystal_") != string::npos) {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
  }

}

// Convert the physical volume name to integer(-like) identifiers
void ExtModule::getVolumeID(const G4TouchableHandle& touch, ExtDetectorID& detID, int& copyID)
{

  // default values
  detID = EXT_UNKNOWN;
  copyID = 0;

  G4String name = touch->GetVolume(0)->GetName();
  if (name.find("CDC") != string::npos) {
    detID = EXT_CDC;
    copyID = touch->GetVolume(0)->GetCopyNo();
  }
  // TOP doesn't have one envelope; it has several "PlacedTOPModule"s
  if (name == "PlacedTOPModule") {
    detID = EXT_TOP;
  }
  // TOP quartz bar (=sensitive) has an automatically generated PV name
  // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
  // YYY is cuttest.
  if (name.find("_cuttest_") != string::npos) {
    detID = EXT_TOP;
    copyID = (touch->GetHistoryDepth() >= 2) ? touch->GetVolume(2)->GetCopyNo() : 0;
  }
  // ARICH has an envelope that contains modules that each contain a moduleSensitive
  if (name == "ARICH.Envelope") {
    detID = EXT_ARICH;
  }
  if (name == "ARICH.DetectorModules") {
    detID = EXT_ARICH;
  }
  if (name == "moduleSensitive") {
    detID = EXT_ARICH;
    copyID = (touch->GetHistoryDepth() >= 1) ? touch->GetVolume(1)->GetCopyNo() : 0;
  }
  // ECL
  if (name == "physicalECL") {
    detID = EXT_ECL;
  }
  // ECL crystal (=sensitive) has an automatically generated PV name
  // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
  // YYY is logicalEcl**Crystal, where **=Br, Fw, or Bw.
  // XXX is 1..144 for Br, 1..16 for Fw, and 1..16 for Bw
  // ZZZ is n_pv_m where n is 1..46 for Br, 1..72 for Fw, and 73..132 for Bw
  // ECL cellID is derived from XXX and n (using code from Poyuan).
  if ((name.find("_logicalEclBrCrystal_") != string::npos) ||
      (name.find("_logicalEclBwCrystal_") != string::npos) ||
      (name.find("_logicalEclFwCrystal_") != string::npos)) {
    detID = EXT_ECL;
    copyID = ECL::ECLGeometryPar::Instance()->ECLVolNameToCellID(name);
  }

}

TMatrixD ExtModule::getCov(const G4ErrorFreeTrajState* state)
{

  G4ErrorFreeTrajParam param = state->GetParameters();
  G4double p = 1.0 / (param.GetInvP() * GeV);   // in GeV/c
  G4double p2 = p * p;
  G4double lambda = param.GetLambda();          // in radians
  G4double phi = param.GetPhi();                // in radians
  G4double sinlambda = sin(lambda);
  G4double coslambda = cos(lambda);
  G4double sinphi = sin(phi);
  G4double cosphi = cos(phi);

  // Transformation Jacobian 6x5 from Geant4e 5x5 to phasespace 6x6
  // Jacobian units are GeV/c, radians, cm
  // Geant4e covariance matrix units are GeV/c, radians, cm (!!!) - see PropagateError()
  // Phase-space covariance matrix units are GeV/c, cm

  G4ErrorMatrix fromGeant4eToPhasespace(6, 5, 0);

  fromGeant4eToPhasespace[0][0] = - p2 * coslambda * cosphi;     // d(px)/d(1/p)
  fromGeant4eToPhasespace[1][0] = - p2 * coslambda * sinphi;     // d(py)/d(1/p)
  fromGeant4eToPhasespace[2][0] = - p2 * sinlambda;              // d(pz)/d(1/p)

  fromGeant4eToPhasespace[0][1] = - p * sinlambda * cosphi;      // d(px)/d(lambda)
  fromGeant4eToPhasespace[1][1] = - p * sinlambda * sinphi;      // d(py)/d(lambda)
  fromGeant4eToPhasespace[2][1] =   p * coslambda;               // d(pz)/d(lambda)

  fromGeant4eToPhasespace[0][2] = - p * coslambda * sinphi;      // d(px)/d(phi)
  fromGeant4eToPhasespace[1][2] =   p * coslambda * cosphi;      // d(py)/d(phi)

  fromGeant4eToPhasespace[3][3] = - sinphi;                      // d(x)/d(yT)
  fromGeant4eToPhasespace[4][3] =   cosphi;                      // d(y)/d(yT)

  fromGeant4eToPhasespace[3][4] = - sinlambda * cosphi;          // d(x)/d(zT)
  fromGeant4eToPhasespace[4][4] = - sinlambda * sinphi;          // d(y)/d(zT)
  fromGeant4eToPhasespace[5][4] =   coslambda;                   // d(z)/d(zT)

  G4ErrorTrajErr covG4e = state->GetError();
  G4ErrorTrajErr phaseSpaceCov = covG4e.similarity(fromGeant4eToPhasespace);
  TMatrixD cov(6, 6);
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      cov[i][j] = phaseSpaceCov[i][j];
    }
  }

  return cov;

}

void ExtModule::getStartPoint(const GFTrack* gfTrack, int pdgCode,
                              G4ThreeVector& position, G4ThreeVector& momentum, G4ErrorTrajErr& covG4e)
{

  GFAbsTrackRep* gfTrackRep = gfTrack->getCardinalRep();
  for (unsigned int rep = 0; rep < gfTrack->getNumReps(); ++rep) {
    if (gfTrack->getTrackRep(rep)->getPDG() == pdgCode) {
      gfTrackRep = gfTrack->getTrackRep(rep);
      break;
    }
  }
  TVectorD firstState(gfTrackRep->getFirstState());
  double charge = (firstState[0] > 0.0 ? 1.0 : -1.0);
  GFDetPlane firstPlane(gfTrackRep->getFirstPlane());
  const TVector3 firstO = firstPlane.getO();
  const TVector3 firstU = firstPlane.getU();
  const TVector3 firstV = firstPlane.getV();
  const TVector3 firstW = firstPlane.getNormal();
  TVector3 firstPosition = firstO + firstState[3] * firstU + firstState[4] * firstV;
  TVector3 firstMomTilde = firstW + firstState[1] * firstU + firstState[2] * firstV;
  if (firstMomTilde * firstPosition < 0.0) { firstMomTilde = -firstMomTilde; }
  TVector3 firstDirection = firstMomTilde.Unit();
  double firstMomMag = 1.0 / fabs(firstState[0]);
  TVector3 firstMomentum = firstMomMag * firstDirection;
  double Bz = GFFieldManager::getInstance()->getFieldVal(TVector3(0, 0, 0)).Z() * kilogauss / tesla;
  double radius = (firstMomentum.Perp() * GeV / eV) / (c_light / (m / s) * charge * Bz) * (m / cm);
  double centerPhi = firstDirection.Phi() - halfpi;
  double centerX = firstPosition.X() + radius * cos(centerPhi);
  double centerY = firstPosition.Y() + radius * sin(centerPhi);
  double pocaPhi = atan2(charge * centerY, charge * centerX) + pi;
  double dPhi = pocaPhi - centerPhi - pi;
  if (dPhi > pi) { dPhi -= twopi; }
  if (dPhi < -pi) { dPhi  += twopi; }
  TVector3 ipPosition(centerX + radius * cos(pocaPhi),
                      centerY + radius * sin(pocaPhi),
                      firstPosition.Z() - dPhi * radius * firstDirection.Z() / firstDirection.Perp());
  TVector3 ipDirection(sin(pocaPhi) * firstDirection.Perp(),
                       -cos(pocaPhi) * firstDirection.Perp(),
                       firstDirection.Z());
  // or, approximately, ipPosition=(0,0,0) and ipDirection=(?,?,firstDirection.Z())
  TVectorD lastState(gfTrackRep->getLastState());
  TMatrixD lastCov(gfTrackRep->getLastCov());
  GFDetPlane lastPlane(gfTrackRep->getLastPlane());
  const TVector3 lastO = lastPlane.getO();
  const TVector3 lastU = lastPlane.getU();
  const TVector3 lastV = lastPlane.getV();
  const TVector3 lastW = lastPlane.getNormal();
  TVector3 lastPosition = lastO + lastState[3] * lastU + lastState[4] * lastV;
  double lastSpu = 1.0;
  TVector3 lastMomTilde = lastW + lastState[1] * lastU + lastState[2] * lastV;
  if (lastMomTilde * lastPosition < 0.0) {
    lastSpu = -lastSpu;
    lastMomTilde = -lastMomTilde;
  }
  double lastMomTildeMag = lastMomTilde.Mag();
  double lastMomTildePerp = lastMomTilde.Perp();
  TVector3 lastDirection = lastMomTilde.Unit();
  double lastMomMag = 1.0 / fabs(lastState[0]);
  TVector3 lastMomentum = lastMomMag * lastDirection;
  double lastPperp = lastMomentum.Perp();
  double sinLambda = lastPperp / lastMomMag;
  double cosLambda = sqrt(1.0 - sinLambda * sinLambda);
  double phi = atan2(lastDirection.Y(), lastDirection.X());
  double sinPhi = sin(phi);
  double cosPhi = cos(phi);

  double pathLength = 0.0;
  double avgW = 0.5 * (ipDirection.Z() + lastDirection.Z());
  if ((fabs(avgW) > 1.0E-10) && (ipDirection.Z()*lastDirection.Z() > 0.0)) {
    pathLength = fabs((lastPosition.Z() - ipPosition.Z()) / avgW);
  } else {
    dPhi = lastDirection.Phi() - ipDirection.Phi();
    if (dPhi < -pi) { dPhi += twopi; }
    if (dPhi >  pi) { dPhi -= twopi; }
    double dx = lastPosition.X() - ipPosition.X();
    double dy = lastPosition.Y() - ipPosition.Y();
    pathLength = sqrt(dx * dx + dy * dy) / (ipDirection.Perp() + lastDirection.Perp())
                 * (dPhi / sin(0.5 * dPhi));
  }
  double mass = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode)->GetPDGMass() / GeV;
  // time of flight from I.P. (ns) at the last point on the Genfit track
  m_tof = pathLength * (sqrt(lastMomMag * lastMomMag + mass * mass) / (lastMomMag * c_light / (cm / ns)));

  // Jacobian matrix d(x,y,z,px,py,pz) / d(q/p,u',v',u,v)
  TMatrixD fromGenfitToPhasespace(6, 5);
  // d(x,y,z)/d(u)
  fromGenfitToPhasespace[0][3] = lastU.X();
  fromGenfitToPhasespace[1][3] = lastU.Y();
  fromGenfitToPhasespace[2][3] = lastU.Z();
  // d(x,y,z)/d(v)
  fromGenfitToPhasespace[0][4] = lastV.X();
  fromGenfitToPhasespace[1][4] = lastV.Y();
  fromGenfitToPhasespace[2][4] = lastV.Z();
  // d(px,py,pz)/d(q/p)
  fromGenfitToPhasespace[3][0] = -charge * lastMomMag * lastMomentum.X();
  fromGenfitToPhasespace[4][0] = -charge * lastMomMag * lastMomentum.Y();
  fromGenfitToPhasespace[5][0] = -charge * lastMomMag * lastMomentum.Z();
  // d(px,py,pz)/d(u')
  double g = lastSpu * lastMomMag / lastMomTildeMag;
  double h = lastMomMag / (lastMomTildeMag * lastMomTildeMag);
  fromGenfitToPhasespace[3][1] = g * lastU.X() - (h * lastState[1]) * lastDirection.X();
  fromGenfitToPhasespace[4][1] = g * lastU.Y() - (h * lastState[1]) * lastDirection.Y();
  fromGenfitToPhasespace[5][1] = g * lastU.Z() - (h * lastState[1]) * lastDirection.Z();
  // d(px,py,pz)/d(v')
  fromGenfitToPhasespace[3][2] = g * lastV.X() - (h * lastState[2]) * lastDirection.X();
  fromGenfitToPhasespace[4][2] = g * lastV.Y() - (h * lastState[2]) * lastDirection.Y();
  fromGenfitToPhasespace[5][2] = g * lastV.Z() - (h * lastState[2]) * lastDirection.Z();
  TMatrixD fromGenfitToPhasespaceT(fromGenfitToPhasespace);
  fromGenfitToPhasespaceT.T();
  TMatrixD lastCovPS = fromGenfitToPhasespace * (lastCov * fromGenfitToPhasespaceT);
  TVector3 lastPosError(sqrt(lastCovPS[0][0]), sqrt(lastCovPS[1][1]), sqrt(lastCovPS[2][2]));
  TVector3 lastDirError(sqrt(lastCovPS[3][3]), sqrt(lastCovPS[4][4]), sqrt(lastCovPS[5][5]));
  lastDirError *= (1.0 / lastMomMag);
  /*
  // Jacobian matrix d(1/p,lambda,phi,Yperp,Zperp)/d(x,y,z,px,py,pz)
  TMatrixD fromPhasespaceToGeant4e(5, 6);
  // d(1/p)/d(px,py,pz)
  fromPhasespaceToGeant4e[0][3] = -lastDirection.X()/(p*p);
  fromPhasespaceToGeant4e[0][4] = -lastDirection.Y()/(p*p);
  fromPhasespaceToGeant4e[0][5] = -lastDirection.Z()/(p*p);
  // d(lambda)/d(px,py,pz)
  fromPhasespaceToGeant4e[1][3] = -lastDirection.X()*lastDirection.Z()/lastPperp;
  fromPhasespaceToGeant4e[1][4] = -lastDirection.Y()*lastDirection.Z()/lastPperp;
  fromPhasespaceToGeant4e[1][5] = lastPperp/(p*p);
  // d(phi)/d(px,py,pz)
  fromPhasespaceToGeant4e[2][3] = -lastMomentum.Y()/(lastPperp*lastPperp);
  fromPhasespaceToGeant4e[2][4] =  lastMomentum.X()/(lastPperp*lastPperp);
  // d(Yperp)/d(x,y,z)
  fromPhasespaceToGeant4e[3][0] = -sinPhi;
  fromPhasespaceToGeant4e[3][1] =  cosPhi;
  // d(Zperp)/d(x,y,z)
  fromPhasespaceToGeant4e[4][0] = -sinLambda*cosPhi;
  fromPhasespaceToGeant4e[4][1] = -sinLambda*sinPhi;
  fromPhasespaceToGeant4e[4][2] =  cosLambda;
  TMatrixD fromPhasespaceToGeant4eT(fromPhasespaceToGeant4e);
  fromPhasespaceToGeant4eT.T();
  TMatrixD lastCovG4e = fromPhasespaceToGeant4e * (lastCovPS * fromPhasespaceToGeant4eT);
  // The 2-step calculation of lastCovG4e above gives the same result as the 1-step calculation below.
  */
  // Jacobian matrix d(1/p,lambda,phi,Yperp,Zperp)/d(q/p,u',v',u,v)
  TMatrixD fromGenfitToGeant4e(5, 5);
  // d(1/p)/d(q/p)
  fromGenfitToGeant4e[0][0] = charge;
  // d(lambda)/d(u')
  fromGenfitToGeant4e[1][1] = lastSpu * (-lastMomTilde.X() * lastMomTilde.Z() * lastU.X() - lastMomTilde.Y() * lastMomTilde.Z() * lastU.Y() + lastMomTildePerp * lastMomTildePerp * lastU.Z()) / (lastMomTildeMag * lastMomTildeMag * lastMomTildePerp);
  // d(lambda)/d(v')
  fromGenfitToGeant4e[1][2] = lastSpu * (-lastMomTilde.X() * lastMomTilde.Z() * lastV.X() - lastMomTilde.Y() * lastMomTilde.Z() * lastV.Y() + lastMomTildePerp * lastMomTildePerp * lastV.Z()) / (lastMomTildeMag * lastMomTildeMag * lastMomTildePerp);
  // d(phi)/d(u')
  fromGenfitToGeant4e[2][1] = lastSpu * (lastMomTilde.X() * lastU.Y() - lastMomTilde.Y() * lastU.X()) / (lastMomTildePerp * lastMomTildePerp);
  // d(phi)/d(v')
  fromGenfitToGeant4e[2][2] = lastSpu * (lastMomTilde.X() * lastV.Y() - lastMomTilde.Y() * lastV.X()) / (lastMomTildePerp * lastMomTildePerp);
  // d(Yperp)/d(u)
  fromGenfitToGeant4e[3][3] = -sinPhi * lastU.X() + cosPhi * lastU.Y();
  // d(Yperp)/d(v)
  fromGenfitToGeant4e[3][4] = -sinPhi * lastV.X() + cosPhi * lastV.Y();
  // d(Zperp)/d(u)
  fromGenfitToGeant4e[4][3] = -sinLambda * cosPhi * lastU.X() - sinLambda * sinPhi * lastU.Y() + cosLambda * lastU.Z();
  // d(Zperp)/d(v)
  fromGenfitToGeant4e[4][4] = -sinLambda * cosPhi * lastV.X() - sinLambda * sinPhi * lastV.Y() + cosLambda * lastV.Z();
  TMatrixD fromGenfitToGeant4eT(fromGenfitToGeant4e);
  fromGenfitToGeant4eT.T();
  TMatrixD lastCovG4e = fromGenfitToGeant4e * (lastCov * fromGenfitToGeant4eT);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      covG4e[i][j] = lastCovG4e[i][j];  // in Geant4e units (GeV, cm)
    }
  }
  position.setX(lastPosition.X()*cm); // in Geant4 units (mm)
  position.setY(lastPosition.Y()*cm);
  position.setZ(lastPosition.Z()*cm);
  momentum.setX(lastMomentum.X()*GeV);  // in Geant4 units (MeV)
  momentum.setY(lastMomentum.Y()*GeV);
  momentum.setZ(lastMomentum.Z()*GeV);

  return;
}

// write another volume-entry or volume-exit point on track
void ExtModule::createHit(const G4ErrorFreeTrajState* state, ExtHitStatus status, int trackID, int pdgCode,
                          StoreArray<ExtHit>& extHits, RelationArray& TrackToExtHits)
{

  G4StepPoint* stepPoint = state->GetG4Track()->GetStep()->GetPreStepPoint();
  G4TouchableHandle preTouch = stepPoint->GetTouchableHandle();
  G4VPhysicalVolume* preVol = preTouch->GetVolume();

  // Perhaps no hit will be stored?
  if (status == EXT_ENTER) {
    if (find(m_enter->begin(), m_enter->end(), preVol) == m_enter->end()) { return; }
  } else if (status == EXT_EXIT) {
    if (find(m_exit->begin(), m_exit->end(), preVol) == m_exit->end()) { return; }
    stepPoint = state->GetG4Track()->GetStep()->GetPostStepPoint();
  }

  TMatrixD phasespacePoint(6, 1);
  TMatrixD covariance(6, 6);
  phasespacePoint[0][0] = stepPoint->GetPosition().x() / cm;
  phasespacePoint[1][0] = stepPoint->GetPosition().y() / cm;
  phasespacePoint[2][0] = stepPoint->GetPosition().z() / cm;
  phasespacePoint[3][0] = stepPoint->GetMomentum().x() / GeV;
  phasespacePoint[4][0] = stepPoint->GetMomentum().y() / GeV;
  phasespacePoint[5][0] = stepPoint->GetMomentum().z() / GeV;
  covariance = getCov(state);
  ExtDetectorID detID(EXT_UNKNOWN);
  int copyID(0);
  getVolumeID(preTouch, detID, copyID);
  TVector3 pos(phasespacePoint[0][0], phasespacePoint[1][0], phasespacePoint[2][0]);
  TVector3 mom(phasespacePoint[3][0], phasespacePoint[4][0], phasespacePoint[5][0]);
  new(extHits.nextFreeAddress()) ExtHit(pdgCode, detID, copyID, status, m_tof, pos, mom, covariance);
  TrackToExtHits.add(trackID, extHits.getEntries() - 1);

}
