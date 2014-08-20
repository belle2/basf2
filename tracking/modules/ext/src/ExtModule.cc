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
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <genfit/Track.h>
#include <genfit/DetPlane.h>
#include <genfit/FieldManager.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>
#include <genfit/Exception.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/MagneticField.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtPhysicsList.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <TMatrixD.h>
#include <TVectorD.h>
#include <TVector3.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <globals.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4UImanager.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4RegionStore.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorPropagator.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4TransportationManager.hh>
#include <G4VPhysicalVolume.hh>
#include <G4FieldManager.hh>

using namespace std;
using namespace Belle2;

REG_MODULE(Ext)

ExtModule::ExtModule() :
  Module(),
  m_ExtMgr(NULL), // initialized later
  m_RunMgr(NULL), // initialized later
  m_TrackingAction(NULL), // initialized later
  m_SteppingAction(NULL), // initialized later
  m_Enter(NULL), // initialized later
  m_Exit(NULL), // initialized later
  m_TOF(0.0), // initialized later
  m_MinRadiusSq(0.0), // initialized later
  m_Target(NULL) // initialized later
{
  m_PDGCode.clear();
  setDescription("Extrapolates tracks from CDC to outer detectors using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("pdgCodes", m_PDGCode, "Positive-charge PDG codes for extrapolation hypotheses", m_PDGCode);
  addParam("TracksColName", m_TracksColName, "Name of collection holding the reconstructed tracks", string("Tracks"));
  addParam("ExtHitsColName", m_ExtHitsColName, "Name of collection holding the ExtHits from the extrapolation", string("ExtHits"));
  addParam("MinPt", m_MinPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated.", double(0.0));
  addParam("MinKE", m_MinKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation.", double(0.002));
  addParam("MaxStep", m_MaxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity).", double(25.0));
  addParam("Cosmic", m_Cosmic, "Particle source (0 = beam, 1 = cosmic ray.", 0);
}

ExtModule::~ExtModule()
{
}

void ExtModule::initialize()
{

  // Convert from GeV to GEANT4 energy units (MeV); avoid negative values
  m_MinPt = max(0.0, m_MinPt) * GeV;
  m_MinKE = max(0.0, m_MinKE) * GeV;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Define the geant4e extrapolation Manager.
  m_ExtMgr = Simulation::ExtManager::GetManager();

  // See if ext will coexist with geant4 simulation and/or muid extrapolation.
  // Only geant4 simulation will have created a G4RunManager singleton; geant4e
  // uses only the G4RunManagerKernel singleton (for ext and/or muid).
  m_RunMgr = G4RunManager::GetRunManager();
  if (m_RunMgr == NULL) {
    B2INFO("Ext will run without simulation")
    m_TrackingAction = NULL;
    m_SteppingAction = NULL;
    if (m_ExtMgr->PrintExtState() == G4String("G4ErrorState_PreInit")) {
      B2INFO("Ext will call InitGeant4e")
      // Create the magnetic field for the geant4e extrapolation
      Simulation::MagneticField* magneticField = new Simulation::MagneticField();
      G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
      fieldManager->SetDetectorField(magneticField);
      fieldManager->CreateChordFinder(magneticField);
      // Tell geant4e about the detector and bare-bones physics list
      m_ExtMgr->SetUserInitialization(new DetectorConstruction());
      G4Region* region = (*(G4RegionStore::GetInstance()))[0];
      region->SetProductionCuts(G4ProductionCutsTable::GetProductionCutsTable()->GetDefaultProductionCuts());
      m_ExtMgr->SetUserInitialization(new Simulation::ExtPhysicsList);
      m_ExtMgr->InitGeant4e();
    } else {
      B2INFO("Ext will not call InitGeant4e since it has already been initialized")
    }
  } else {
    B2INFO("Ext will coexist with simulation")
    m_TrackingAction = const_cast<G4UserTrackingAction*>(m_RunMgr->GetUserTrackingAction());
    m_SteppingAction = const_cast<G4UserSteppingAction*>(m_RunMgr->GetUserSteppingAction());
    if (m_ExtMgr->PrintExtState() == G4String("G4ErrorState_PreInit")) {
      B2INFO("Ext will call InitGeant4e")
      if (m_ExtMgr->PrintG4State() == G4String("G4State_Idle")) {
        // ... if G4RunManager::RunInitialization() will be called via fullsim/beginRun()
        m_ExtMgr->InitGeant4e();
        G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
      } else {
        // ... if G4RunManager::RunInitialization() has been called via fullsim/initialize()
        G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
        m_ExtMgr->InitGeant4e();
      }
    } else {
      B2INFO("Ext will not call InitGeant4e since it has already been initialized")
    }
  }

  // Redefine ext's step length, magnetic field step limitation (fraction of local curvature radius),
  // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
  G4double maxStep = ((m_MaxStep == 0.0) ? 10.0 : std::min(10.0, m_MaxStep)) * cm;
  char stepSize[80];
  std::sprintf(stepSize, "/geant4e/limits/stepLength %8.2f mm", maxStep);
  G4UImanager::GetUIpointer()->ApplyCommand(stepSize);
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  GearDir strContent = GearDir("Detector/DetectorComponent[@name=\"COIL\"]/Content/");
  double offsetZ = strContent.getLength("OffsetZ") * cm;
  double rMaxCoil = strContent.getLength("Cryostat/Rmin") * cm;
  double halfLength = strContent.getLength("Cryostat/HalfLength") * cm;
  m_Target = new Simulation::ExtCylSurfaceTarget(rMaxCoil, offsetZ - halfLength, offsetZ + halfLength);
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_Target);
  m_MinRadiusSq = (rMaxCoil * 0.25) * (rMaxCoil * 0.25); // roughly 40 cm

  // Hypotheses for extrapolation
  if (m_PDGCode.empty()) {
    m_ChargedStable.push_back(Const::pion);
    m_ChargedStable.push_back(Const::electron);
    m_ChargedStable.push_back(Const::muon);
    m_ChargedStable.push_back(Const::kaon);
    m_ChargedStable.push_back(Const::proton);
  } else { // user defined
    std::vector<Const::ChargedStable> stack;
    stack.push_back(Const::pion);
    stack.push_back(Const::electron);
    stack.push_back(Const::muon);
    stack.push_back(Const::kaon);
    stack.push_back(Const::proton);
    for (unsigned i = 0; i < m_PDGCode.size(); ++i) {
      for (unsigned k = 0; k < stack.size(); ++k) {
        if (abs(m_PDGCode[i]) == stack[k].getPDGCode()) {
          m_ChargedStable.push_back(stack[k]);
          stack.erase(stack.begin() + k);
          --k;
        }
      }
    }
    if (m_ChargedStable.empty()) B2ERROR("No valid PDG codes for extrapolation")
    }

  for (unsigned i = 0; i < m_ChargedStable.size(); ++i) {
    B2INFO("Hypothesis for PDG code " << m_ChargedStable[i].getPDGCode() << " and its antiparticle will be extrapolated");
  }

  // Register output and relation arrays
  StoreArray<ExtHit> extHits(m_ExtHitsColName);
  StoreArray<Track> tracks(m_TracksColName);
  extHits.registerInDataStore();
  tracks.registerRelationTo(extHits);

  return;

}

void ExtModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("ext: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun())
}

void ExtModule::event()
{

  // Put geant4 in proper state (in case this module is in a separate process)
  if (m_ExtMgr->PrintG4State() == "G4State_Idle") {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  // Disable simulation-specific actions temporarily while we extrapolate
  if (m_RunMgr) {
    m_RunMgr->SetUserAction((G4UserTrackingAction*)NULL);
    m_RunMgr->SetUserAction((G4UserSteppingAction*)NULL);
  }

  // Loop over the reconstructed tracks.
  // Do extrapolation for each hypotheses (pion, electron, muon, kaon, proton)
  // of each reconstructed track.
  // Pion hypothesis:  extrapolate until calorimeter exit
  // Other hypotheses: extrapolate up to but not including calorimeter

  StoreArray<Track> tracks(m_TracksColName);

  G4ThreeVector position;
  G4ThreeVector momentum;
  G4ErrorTrajErr covG4e(5, 0);

  for (int t = 0; t < tracks.getEntries(); ++t) {

    for (unsigned int hypothesis = 0; hypothesis < m_ChargedStable.size(); ++hypothesis) {

      Const::ChargedStable chargedStable = m_ChargedStable[hypothesis];

      const TrackFitResult* trackFit = tracks[t]->getTrackFitResult(chargedStable);
      if (!trackFit) {
        B2ERROR("No valid TrackFitResult for PDGcode " <<
                chargedStable.getPDGCode() << ": extrapolation not possible")
        continue;
      }

      const genfit::Track* gfTrack = DataStore::getRelated<genfit::Track>(trackFit);
      if (!gfTrack) {
        B2ERROR("No relation of TrackFitResult with genfit::Track for PDGcode " <<
                chargedStable.getPDGCode() << ": extrapolation not possible")
        continue;
      }

      int charge = int(gfTrack->getFitStatus(gfTrack->getCardinalRep())->getCharge());
      int pdgCode = chargedStable.getPDGCode() * charge;
      if (chargedStable == Const::electron || chargedStable == Const::muon) pdgCode = -pdgCode;

      getStartPoint(gfTrack, pdgCode, position, momentum, covG4e);
      if (momentum.perp() <= m_MinPt) continue;
      if (m_Target->GetDistanceFromPoint(position) < 0.0) continue;
      G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
      string g4eName = "g4e_" + particle->GetParticleName();
      double mass = particle->GetPDGMass();
      double minP = sqrt((mass + m_MinKE) * (mass + m_MinKE) - mass * mass);
      G4ErrorFreeTrajState* state = new G4ErrorFreeTrajState(g4eName, position, momentum, covG4e);
      m_ExtMgr->InitTrackPropagation();
      while (true) {
        const G4int    errCode    = m_ExtMgr->PropagateOneStep(state, G4ErrorMode_PropForwards);
        G4Track*       track      = state->GetG4Track();
        const G4Step*  step       = track->GetStep();
        const G4int    preStatus  = step->GetPreStepPoint()->GetStepStatus();
        const G4int    postStatus = step->GetPostStepPoint()->GetStepStatus();
        // First step on this track?
        if (preStatus == fUndefined) {
          createHit(state, EXT_FIRST, tracks[t], pdgCode);
        }
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (step->GetStepLength() > 0.0) {
          if (preStatus == fGeomBoundary) {      // first step in this volume?
            createHit(state, EXT_ENTER, tracks[t], pdgCode);
          }
          m_TOF += step->GetDeltaTime();
          // Last step in this volume?
          if (postStatus == fGeomBoundary) {
            createHit(state, EXT_EXIT, tracks[t], pdgCode);
          }
        }
        // Post-step momentum too low?
        if (errCode || (track->GetMomentum().mag() < minP)) {
          createHit(state, EXT_STOP, tracks[t], pdgCode);
          break;
        }
        if (m_Target->GetDistanceFromPoint(track->GetPosition()) < 0.0) {
          createHit(state, EXT_ESCAPE, tracks[t], pdgCode);
          break;
        }
        // Stop extrapolating as soon as the track curls inward too much
        if (track->GetPosition().perp2() < m_MinRadiusSq) break;
      } // track-extrapolation "infinite" loop

      m_ExtMgr->EventTermination();

      delete state;

    } // hypothesis loop

  } // track loop

  if (m_RunMgr) {
    m_RunMgr->SetUserAction(m_TrackingAction);
    m_RunMgr->SetUserAction(m_SteppingAction);
  }

}

void ExtModule::endRun()
{
}

void ExtModule::terminate()
{

  if (m_RunMgr) {
    m_RunMgr->SetUserAction(m_TrackingAction);
    m_RunMgr->SetUserAction(m_SteppingAction);
  }
  delete m_Target;
  delete m_Enter;
  delete m_Exit;

}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void ExtModule::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("No geometry defined. Please create the geometry first.")
  }

  m_Enter = new vector<G4VPhysicalVolume*>;
  m_Exit  = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // TOP doesn't have one envelope; it has several "PlacedTOPModule"s
    if (name == "PlacedTOPModule") {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    // TOP quartz bar (=sensitive) has an automatically generated PV name
    // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
    // YYY is cuttest.
    if (name.find("_cuttest_") != string::npos) {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    if (name == "ARICH.AerogelSupportPlate") {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    if (name == "moduleSensitive") {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    // ECL
    if (name == "physicalECL") {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    // ECL crystal (=sensitive) has an automatically generated PV name
    // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
    // YYY is logicalEcl**Crystal, where **=Br, Fw, or Bw.
    // XXX is 1..144 for Br, 1..16 for Fw, and 1..16 for Bw
    // ZZZ is n_pv_m where n is 1..46 for Br, 1..72 for Fw, and 73..132 for Bw
    // CopyNo() encodes XXX and n.
    if (name.find("_logicalEclBrCrystal_") != string::npos) {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    if (name.find("_logicalEclFwCrystal_") != string::npos) {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
    if (name.find("_logicalEclBwCrystal_") != string::npos) {
      m_Enter->push_back(*iVol);
      m_Exit->push_back(*iVol);
    }
  }

}

// Convert the physical volume name to integer(-like) identifiers
void ExtModule::getVolumeID(const G4TouchableHandle& touch, Const::EDetector& detID, int& copyID)
{

  // default values
  detID = Const::EDetector::invalidDetector;
  copyID = 0;

  G4String name = touch->GetVolume(0)->GetName();
  if (name.find("CDC") != string::npos) {
    detID = Const::EDetector::CDC;
    copyID = touch->GetVolume(0)->GetCopyNo();
  }
  // TOP doesn't have one envelope; it has several "PlacedTOPModule"s
  if (name == "PlacedTOPModule") {
    detID = Const::EDetector::TOP;
  }
  // TOP quartz bar (=sensitive) has an automatically generated PV name
  // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
  // YYY is cuttest.
  if (name.find("_cuttest_") != string::npos) {
    detID = Const::EDetector::TOP;
    copyID = (touch->GetHistoryDepth() >= 2) ? touch->GetVolume(2)->GetCopyNo() : 0;
  }
  // ARICH has an envelope that contains modules that each contain a moduleSensitive
  if (name == "ARICH.AerogelSupportPlate") {
    detID = Const::EDetector::ARICH;
    copyID = 12345;
  }
  if (name == "moduleSensitive") {
    detID = Const::EDetector::ARICH;
    copyID = (touch->GetHistoryDepth() >= 1) ? touch->GetVolume(1)->GetCopyNo() : 0;
  }
  // ECL
  if (name == "physicalECL") {
    detID = Const::EDetector::ECL;
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
    detID = Const::EDetector::ECL;
    copyID = ECL::ECLGeometryPar::Instance()->ECLVolNameToCellID(name);
  }

}

void ExtModule::getStartPoint(const genfit::Track* gfTrack, int pdgCode,
                              G4ThreeVector& position, G4ThreeVector& momentum, G4ErrorTrajErr& covG4e)
{

  genfit::AbsTrackRep* gfTrackRep = gfTrack->getCardinalRep();
  for (unsigned int rep = 0; rep < gfTrack->getNumReps(); ++rep) {
    if (gfTrack->getTrackRep(rep)->getPDG() == pdgCode) {
      gfTrackRep = gfTrack->getTrackRep(rep);
      break;
    }
  }

  bool firstLast = true; // for genfit exception catch
  try {
    const genfit::TrackPoint* firstPoint = gfTrack->getPointWithMeasurementAndFitterInfo(0, gfTrackRep);
    const genfit::AbsFitterInfo* firstFitterInfo = firstPoint->getFitterInfo(gfTrackRep);
    const genfit::MeasuredStateOnPlane& firstState = firstFitterInfo->getFittedState(true);
    TVector3 firstPosition, firstMomentum;
    TMatrixDSym firstCov(6);
    gfTrackRep->getPosMomCov(firstState, firstPosition, firstMomentum, firstCov);
    int charge = gfTrackRep->getCharge(firstState);
    TVector3 firstDirection(firstMomentum.Unit());

    double Bz = genfit::FieldManager::getInstance()->getFieldVal(TVector3(0, 0, 0)).Z() * kilogauss / tesla;
    double radius = (firstMomentum.Perp() * GeV / eV) / (c_light / (m / s) * charge * Bz) * (m / cm);
    double centerPhi = firstMomentum.Phi() - halfpi;
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
    firstLast = false;
    const genfit::TrackPoint* lastPoint = gfTrack->getPointWithMeasurementAndFitterInfo(-1, gfTrackRep);
    const genfit::AbsFitterInfo* lastFitterInfo = lastPoint->getFitterInfo(gfTrackRep);
    const genfit::MeasuredStateOnPlane& lastState = lastFitterInfo->getFittedState(true);
    TVector3 lastPosition, lastMomentum;
    TMatrixDSym lastCov(6);
    gfTrackRep->getPosMomCov(lastState, lastPosition, lastMomentum, lastCov);
    TVector3 lastDirection(lastMomentum.Unit());
    double lastMomMag = lastMomentum.Mag();

    // The path length should really be taken from the fit result ...
    // ... but leave this as is for exact comparison.
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
    m_TOF = pathLength * (sqrt(lastMomMag * lastMomMag + mass * mass) / (lastMomMag * c_light / (cm / ns)));

    covG4e = fromPhasespaceToG4e(lastMomentum, lastCov); // in Geant4e units (GeV/c, cm)
    position.setX(lastPosition.X()*cm); // in Geant4 units (mm)
    position.setY(lastPosition.Y()*cm);
    position.setZ(lastPosition.Z()*cm);
    momentum.setX(lastMomentum.X()*GeV);  // in Geant4 units (MeV/c)
    momentum.setY(lastMomentum.Y()*GeV);
    momentum.setZ(lastMomentum.Z()*GeV);
  }

  catch (genfit::Exception& e) {
    B2WARNING("Caught genfit exception for " << (firstLast ? "first" : "last") << " point on track; will not extrapolate. " << e.what())
    // Do not extrapolate this track by forcing minPt cut to fail
    momentum.setX(0.0);
    momentum.setY(0.0);
    momentum.setZ(0.0);
  }

  return;
}

TMatrixDSym ExtModule::fromG4eToPhasespace(const G4ErrorFreeTrajState* state)
{

  // Convert Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in GeV/c, radians, cm)
  // to phase-space covariance matrix with parameters x, y, z, px, py, pz (in GeV/c, cm)
  // (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
  // phi = atan( py / px )
  // lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )
  // xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
  // yT = -x * sin(phi) + y * cos(phi)
  // zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)

  G4ErrorFreeTrajParam param = state->GetParameters();
  double p = 1.0 / (param.GetInvP() * GeV);     // in GeV/c
  double p2 = p * p;
  double lambda = param.GetLambda();            // in radians
  double phi = param.GetPhi();          // in radians
  double sinlambda = sin(lambda);
  double coslambda = cos(lambda);
  double sinphi = sin(phi);
  double cosphi = cos(phi);

  // Transformation Jacobian 6x5 from Geant4e 5x5 to phase-space 6x6

  G4ErrorMatrix jacobian(6, 5, 0);

  jacobian(4, 1) = - p2 * coslambda * cosphi;           // @(px)/@(1/p)
  jacobian(5, 1) = - p2 * coslambda * sinphi;           // @(py)/@(1/p)
  jacobian(6, 1) = - p2 * sinlambda;                    // @(pz)/@(1/p)

  jacobian(4, 2) = - p * sinlambda * cosphi;            // @(px)/@(lambda)
  jacobian(5, 2) = - p * sinlambda * sinphi;            // @(py)/@(lambda)
  jacobian(6, 2) =   p * coslambda;                     // @(pz)/@(lambda)

  jacobian(4, 3) = - p * coslambda * sinphi;            // @(px)/@(phi)
  jacobian(5, 3) =   p * coslambda * cosphi;            // @(py)/@(phi)

  jacobian(1, 4) = - sinphi;                            // @(x)/@(yT)
  jacobian(2, 4) =   cosphi;                            // @(y)/@(yT)

  jacobian(1, 5) = - sinlambda * cosphi;                // @(x)/@(zT)
  jacobian(2, 5) = - sinlambda * sinphi;                // @(y)/@(zT)
  jacobian(3, 5) =   coslambda;                         // @(z)/@(zT)

  G4ErrorTrajErr g4eCov = state->GetError();
  G4ErrorSymMatrix phasespaceCov = g4eCov.similarity(jacobian);

  TMatrixDSym covariance(6);
  for (int k = 0; k < 6; k++) {
    for (int j = 0; j < 6; j++) {
      covariance[j][k] = phasespaceCov[j][k];
    }
  }
  return covariance;

}

G4ErrorTrajErr ExtModule::fromPhasespaceToG4e(const TVector3& momentum, const TMatrixDSym& covariance)
{

  // Convert phase-space covariance matrix with parameters x, y, z, px, py, pz (in GeV/c, cm)
  // to Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in GeV/c, radians, cm)
  // xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
  // yT = -x * sin(phi) + y * cos(phi)
  // zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)
  // (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
  // phi = atan( py / px )
  // lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )

  G4ErrorSymMatrix temp(6, 0);
  for (int k = 0; k < 6; k++) {
    for (int j = k; j < 6; j++) {
      temp[j][k] = covariance[j][k];
    }
  }

  double sinLambda = momentum.CosTheta();
  double cosLambda = sqrt(1.0 - sinLambda * sinLambda);
  double phi = momentum.Phi();
  double cosPhi = cos(phi);
  double sinPhi = sin(phi);
  double pInvSq = 1.0 / momentum.Mag2();
  double pInv   = sqrt(pInvSq);
  double pPerpInv = 1.0 / momentum.Perp();

  // Transformation Jacobian 5x6 from phase-space 6x6 to Geant4e 5x5
  G4ErrorMatrix jacobian(5, 6, 0);

  jacobian(1, 4) = - pInvSq * cosLambda * cosPhi;       // @(1/p)/@(px)
  jacobian(1, 5) = - pInvSq * cosLambda * sinPhi;       // @(1/p)/@(py)
  jacobian(1, 6) = - pInvSq * sinLambda;                // @(1/p)/@(pz)

  jacobian(2, 4) = - pInv * sinLambda * cosPhi;         // @(lambda)/@(px)
  jacobian(2, 5) = - pInv * sinLambda * sinPhi;         // @(lambda)/@(py)
  jacobian(2, 6) =   pInv * cosLambda;                  // @(lambda)/@(pz)

  jacobian(3, 4) = - pPerpInv * sinPhi;                 // @(phi)/@(px)
  jacobian(3, 5) =   pPerpInv * cosPhi;                 // @(phi)/@(py)

  jacobian(4, 1) = - sinPhi;                            // @(yT)/@(x)
  jacobian(4, 2) =   cosPhi;                            // @(yT)/@(y)

  jacobian(5, 1) = - sinLambda * cosPhi;                // @(zT)/@(x)
  jacobian(5, 2) = - sinLambda * sinPhi;                // @(zT)/@(y)
  jacobian(5, 3) =   cosLambda;                         // @(zT)/@(z)

  return temp.similarity(jacobian);

}

// write another volume-entry or volume-exit point on track
void ExtModule::createHit(const G4ErrorFreeTrajState* state, ExtHitStatus status, Track* track, int pdgCode)
{

  StoreArray<ExtHit> extHits(m_ExtHitsColName);

  G4StepPoint* stepPoint = state->GetG4Track()->GetStep()->GetPreStepPoint();
  G4TouchableHandle preTouch = stepPoint->GetTouchableHandle();
  G4VPhysicalVolume* preVol = preTouch->GetVolume();

  // Perhaps no hit will be stored?
  if (status == EXT_ENTER) {
    if (find(m_Enter->begin(), m_Enter->end(), preVol) == m_Enter->end()) { return; }
  } else if (status == EXT_EXIT) {
    if (find(m_Exit->begin(), m_Exit->end(), preVol) == m_Exit->end()) { return; }
    stepPoint = state->GetG4Track()->GetStep()->GetPostStepPoint();
  }

  TVector3 pos(stepPoint->GetPosition().x() / cm, stepPoint->GetPosition().y() / cm, stepPoint->GetPosition().z() / cm);
  TVector3 mom(stepPoint->GetMomentum().x() / GeV, stepPoint->GetMomentum().y() / GeV, stepPoint->GetMomentum().z() / GeV);
  Const::EDetector detID(Const::EDetector::invalidDetector);
  int copyID(0);
  getVolumeID(preTouch, detID, copyID);
  ExtHit* extHit = extHits.appendNew(pdgCode, detID, copyID, status, m_TOF, pos, mom, fromG4eToPhasespace(state));
  track->addRelationTo(extHit);

}
