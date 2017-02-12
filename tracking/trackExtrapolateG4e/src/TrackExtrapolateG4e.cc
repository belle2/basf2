/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackExtrapolateG4e/TrackExtrapolateG4e.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/DetPlane.h>
#include <genfit/FieldManager.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>
#include <genfit/Exception.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/ExtManager.h>
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
#include <G4VPhysicalVolume.hh>
#include <G4ParticleTable.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4UImanager.hh>

#define TWOPI (2.0*M_PI)

using namespace std;
using namespace Belle2;

TrackExtrapolateG4e* TrackExtrapolateG4e::m_Singleton = NULL;

TrackExtrapolateG4e* TrackExtrapolateG4e::GetInstance()
{
  if (m_Singleton == NULL) m_Singleton = new TrackExtrapolateG4e;
  return m_Singleton;
}

TrackExtrapolateG4e::TrackExtrapolateG4e() :
  m_MinPt(0.0), // initialized later
  m_MinKE(0.0), // initialized later
  m_TracksColName(""), // initialized later
  m_ExtHitsColName(""), // initialized later
  m_ExtMgr(NULL), // initialized later
  m_BKLMVolumes(NULL), // initialized later
  m_EKLMVolumes(NULL), // initialized later
  m_EnterExit(NULL), // initialized later
  m_MinRadiusSq(0.0), // initialized later
  m_Target(NULL) // initialized later
{
  m_ChargedStable.clear(); // initialized later
}

TrackExtrapolateG4e::~TrackExtrapolateG4e()
{
}

void TrackExtrapolateG4e::initialize(const std::vector<int>& pdgCodes,
                                     const std::string& tracksColName,
                                     const std::string& extHitsColName,
                                     double minPt,
                                     double minKE,
                                     double maxStep,
                                     int cosmic,
                                     int trackingVerbosity,
                                     bool enableVisualization,
                                     const std::string& magneticFieldStepperName,
                                     double magneticCacheDistance,
                                     double deltaChordInMagneticField,
                                     const std::vector<std::string>& uiCommands)
{

  m_TracksColName = tracksColName;
  m_ExtHitsColName = extHitsColName;

  // Convert from GeV to GEANT4 energy units (MeV); avoid negative values
  m_MinPt = max(0.0, minPt) * CLHEP::GeV;
  m_MinKE = max(0.0, minKE) * CLHEP::GeV;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Initialize the (singleton) extrapolation manager.  It will check
  // whether it will run with or without FullSimModule and with or without
  // Muid (or any other geant4e-based extrapolation module).
  m_ExtMgr = Simulation::ExtManager::GetManager();
  m_ExtMgr->Initialize("Ext", magneticFieldStepperName, magneticCacheDistance, deltaChordInMagneticField,
                       enableVisualization, trackingVerbosity, uiCommands);

  // Redefine ext's step length, magnetic field step limitation (fraction of local curvature radius),
  // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
  maxStep = ((maxStep == 0.0) ? 10.0 : std::min(10.0, maxStep)) * CLHEP::cm;
  char stepSize[80];
  std::sprintf(stepSize, "/geant4e/limits/stepLength %8.2f mm", maxStep);
  G4UImanager::GetUIpointer()->ApplyCommand(stepSize);
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  m_Cosmic = cosmic; // DIVOT: not used yet

  GearDir coilContent = GearDir("Detector/DetectorComponent[@name=\"COIL\"]/Content/");
  double offsetZ = coilContent.getLength("OffsetZ") * CLHEP::cm;
  double rMaxCoil = coilContent.getLength("Cryostat/Rmin") * CLHEP::cm;
  double halfLength = coilContent.getLength("Cryostat/HalfLength") * CLHEP::cm;
  m_Target = new Simulation::ExtCylSurfaceTarget(rMaxCoil, offsetZ - halfLength, offsetZ + halfLength);
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_Target);
  GearDir beampipeContent = GearDir("Detector/DetectorComponent[@name=\"BeamPipe\"]/Content/");
  double beampipeRadius = beampipeContent.getLength("Lv2OutBe/R2") * CLHEP::cm; // mm
  m_MinRadiusSq = beampipeRadius * beampipeRadius; // mm^2

  // Hypotheses for extrapolation
  const Const::ParticleSet set = Const::chargedStableSet;
  if (pdgCodes.empty()) {
    for (const Const::ChargedStable& pdgIter : set) {
      m_ChargedStable.push_back(pdgIter);
    }
  } else { // user defined
    std::vector<Const::ChargedStable> stack;
    for (const Const::ChargedStable& pdgIter : set) {
      stack.push_back(pdgIter);
    }
    for (unsigned i = 0; i < pdgCodes.size(); ++i) {
      for (unsigned k = 0; k < stack.size(); ++k) {
        if (abs(pdgCodes[i]) == stack[k].getPDGCode()) {
          m_ChargedStable.push_back(stack[k]);
          stack.erase(stack.begin() + k);
          --k;
        }
      }
    }
    if (m_ChargedStable.empty()) B2ERROR("No valid PDG codes for extrapolation");
  }

  for (unsigned i = 0; i < m_ChargedStable.size(); ++i) {
    B2INFO("Ext hypothesis for PDG code " << m_ChargedStable[i].getPDGCode() << " and its antiparticle will be extrapolated");
  }

  // Register output and relation arrays
  StoreArray<ExtHit> extHits(m_ExtHitsColName);
  StoreArray<Track> tracks(m_TracksColName);
  extHits.registerInDataStore();
  tracks.registerRelationTo(extHits);

}

void TrackExtrapolateG4e::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("ext: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun());
}

void TrackExtrapolateG4e::event(bool isMuid)
{

  // Put geant4 in proper state (in case this module is in a separate process)
  if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  // Loop over the reconstructed tracks.
  // Do extrapolation for each hypothesis (pion, electron, muon, kaon, proton,
  // deuteron) of each reconstructed track until calorimeter exit.

  StoreArray<Track> tracks(m_TracksColName);

  G4ThreeVector positionG4e;
  G4ThreeVector momentumG4e;
  G4ErrorTrajErr covG4e(5, 0);

  for (auto& b2track : tracks) {

    for (const auto& chargedStable : m_ChargedStable) {

      RecoTrack* recoTrack = b2track.getRelatedTo<RecoTrack>();

      // TODO: Correct rep! Check, if really fitted!
      const genfit::AbsTrackRep* trackRep = recoTrack->getCardinalRepresentation();

      int charge = int(recoTrack->getTrackFitStatus(trackRep)->getCharge());
      int pdgCode = chargedStable.getPDGCode() * charge;
      if (chargedStable == Const::electron || chargedStable == Const::muon) pdgCode = -pdgCode;
      if (pdgCode == 0) {
        B2WARNING("Skipping track. PDGCode " << pdgCode << " is zero, probably because charge was zero (charge=" << charge << ").");
        continue;
      }

      double tof = 0.0;
      getStartPoint(recoTrack, trackRep, pdgCode, positionG4e, momentumG4e, covG4e, tof);
      if (momentumG4e.perp() <= m_MinPt) continue;
      if (m_Target->GetDistanceFromPoint(positionG4e) < 0.0) continue;
      G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
      string g4eName = "g4e_" + particle->GetParticleName();
      double mass = particle->GetPDGMass();
      double minP = sqrt((mass + m_MinKE) * (mass + m_MinKE) - mass * mass);
      G4ErrorFreeTrajState* g4eState = new G4ErrorFreeTrajState(g4eName, positionG4e, momentumG4e, covG4e);
      m_ExtMgr->InitTrackPropagation();
      ExtState extState = { isMuid, pdgCode, g4eState, &b2track, m_ExtHitsColName, tof };
      while (true) {
        const G4int errCode = m_ExtMgr->PropagateOneStep(g4eState);
        G4Track*       track      = g4eState->GetG4Track();
        const G4Step*  step       = track->GetStep();
        const G4int    preStatus  = step->GetPreStepPoint()->GetStepStatus();
        const G4int    postStatus = step->GetPostStepPoint()->GetStepStatus();
        // First step on this track?
        if (preStatus == fUndefined) {
          createHit(EXT_FIRST, extState);
        }
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (step->GetStepLength() > 0.0) {
          if (preStatus == fGeomBoundary) {      // first step in this volume?
            createHit(EXT_ENTER, extState);
          }
          extState.tof += step->GetDeltaTime();
          // Last step in this volume?
          if (postStatus == fGeomBoundary) {
            createHit(EXT_EXIT, extState);
          }
        }
        // Post-step momentum too low?
        if (errCode || (track->GetMomentum().mag() < minP)) {
          createHit(EXT_STOP, extState);
          break;
        }
        // Detect escapes from the imaginary target cylinder.
        if (m_Target->GetDistanceFromPoint(track->GetPosition()) < 0.0) {
          createHit(EXT_ESCAPE, extState);
          break;
        }
        // Stop extrapolating as soon as the track curls inward too much
        if (track->GetPosition().perp2() < m_MinRadiusSq) {
          break;
        }
      } // track-extrapolation "infinite" loop

      m_ExtMgr->EventTermination();

      delete g4eState;

    } // hypothesis loop

  } // track loop

}

void TrackExtrapolateG4e::extrapolate(int pdgCode, // signed for charge
                                      double tof, // in ns (from IP to position)
                                      const G4ThreeVector& position, // in cm (genfit2 units)
                                      const G4ThreeVector& momentum, // in GeV/c (genfit2 units)
                                      const G4ErrorSymMatrix& covariance, // (6x6) using cm, GeV/c (genfit2 units)
                                      const std::string& extHitsColName)
{

  // Put geant4 in proper state (in case this module is in a separate process)
  if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  // Do extrapolation for selected hypothesis (pion, electron, muon, kaon, proton,
  // deuteron) for the selected track until calorimeter exit.

  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
  string g4eName = "g4e_" + particle->GetParticleName();
  double mass = particle->GetPDGMass();
  double minP = sqrt((mass + m_MinKE) * (mass + m_MinKE) - mass * mass);
  G4ThreeVector positionG4e = position * CLHEP::cm; // convert from genfit2 units (cm) to geant4 units (mm)
  G4ThreeVector momentumG4e = momentum * CLHEP::GeV; // convert from genfit2 units (GeV/c) to geant4 units (MeV/c)
  if (momentum.perp() <= m_MinPt) return;
  if (m_Target->GetDistanceFromPoint(position) < 0.0) return;
  G4ErrorSymMatrix covG4e = fromPhasespaceToG4e(momentum, covariance); // in Geant4e units (GeV/c, cm)
  G4ErrorFreeTrajState* g4eState = new G4ErrorFreeTrajState(g4eName, positionG4e, momentumG4e, covG4e);
  ExtState extState = { false, pdgCode, g4eState, NULL, extHitsColName, tof };
  m_ExtMgr->InitTrackPropagation();

  while (true) {
    const G4int errCode = m_ExtMgr->PropagateOneStep(g4eState);
    G4Track*       track      = g4eState->GetG4Track();
    const G4Step*  step       = track->GetStep();
    const G4int    preStatus  = step->GetPreStepPoint()->GetStepStatus();
    const G4int    postStatus = step->GetPostStepPoint()->GetStepStatus();
    // First step on this track?
    if (preStatus == fUndefined) {
      createHit(EXT_FIRST, extState);
    }
    // Ignore the zero-length step by PropagateOneStep() at each boundary
    if (step->GetStepLength() > 0.0) {
      if (preStatus == fGeomBoundary) {      // first step in this volume?
        createHit(EXT_ENTER, extState);
      }
      extState.tof += step->GetDeltaTime();
      // Last step in this volume?
      if (postStatus == fGeomBoundary) {
        createHit(EXT_EXIT, extState);
      }
    }
    // Post-step momentum too low?
    if (errCode || (track->GetMomentum().mag() < minP)) {
      createHit(EXT_STOP, extState);
      break;
    }
    // Detect escapes from the imaginary target cylinder.
    if (m_Target->GetDistanceFromPoint(track->GetPosition()) < 0.0) {
      createHit(EXT_ESCAPE, extState);
      break;
    }
    // Stop extrapolating as soon as the track curls inward too much
    if (track->GetPosition().perp2() < m_MinRadiusSq) {
      break;
    }
  } // track-extrapolation "infinite" loop

  m_ExtMgr->EventTermination();

  delete g4eState;

}

void TrackExtrapolateG4e::endRun()
{
}

void TrackExtrapolateG4e::terminate()
{

  delete m_Target;
  delete m_BKLMVolumes;
  delete m_EKLMVolumes;
  delete m_EnterExit;
  m_ExtMgr->RunTermination();
}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void TrackExtrapolateG4e::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("No geometry defined. Please create the geometry first.");
  }

  m_BKLMVolumes = new vector<G4VPhysicalVolume*>;
  m_EKLMVolumes = new vector<G4VPhysicalVolume*>;
  m_EnterExit = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // TOP doesn't have one envelope; it has 16 "TOPModule"s
    if (name.find("TOPModule") != string::npos) {
      m_EnterExit->push_back(*iVol);
    }
    // TOP quartz bar (=sensitive) has an automatically generated PV name
    // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
    // YYY is as below
    else if (name.find("_TOPPrism_") != string::npos or
             name.find("_TOPBarSegment") != string::npos or
             name.find("_TOPMirrorSegment") != string::npos or
             name.find("TOPBarSegment1Glue") != string::npos or
             name.find("TOPBarSegment2Glue") != string::npos or
             name.find("TOPMirrorSegmentGlue") != string::npos) {
      m_EnterExit->push_back(*iVol);
    } else if (name == "ARICH.AerogelSupportPlate") {
      m_EnterExit->push_back(*iVol);
    } else if (name == "moduleWindow") {
      m_EnterExit->push_back(*iVol);
    }
    // ECL crystal
    else if (name.find("lv_barrel_crystal_") != string::npos ||
             name.find("lv_forward_crystal_") != string::npos ||
             name.find("lv_backward_crystal_") != string::npos) {
      m_EnterExit->push_back(*iVol);
    }
    // Barrel KLM: BKLM.Layer**GasPhysical for RPCs or BKLM.Layer**ChimneyGasPhysical for RPCs
    //             BKLM.ScintActiveType*Physical for scintillator strips
    else if (name.compare(0, 5, "BKLM.") == 0) {
      if ((name.find("ScintActiveType") != string::npos) ||
          (name.find("GasPhysical") != string::npos)) {
        m_BKLMVolumes->push_back(*iVol);
      }
    }
    // Endcap KLM: StripSensitive_*
    else if (name.compare(0, 14, "StripSensitive") == 0) {
      m_EKLMVolumes->push_back(*iVol);
    }
  }

}

// Convert the physical volume to integer(-like) identifiers
void TrackExtrapolateG4e::getVolumeID(const G4TouchableHandle& touch, Const::EDetector& detID, int& copyID)
{

  // default values
  detID = Const::EDetector::invalidDetector;
  copyID = 0;

  G4String name = touch->GetVolume(0)->GetName();
  if (name.find("CDC") != string::npos) {
    detID = Const::EDetector::CDC;
    copyID = touch->GetVolume(0)->GetCopyNo();
  } else if (name.find("TOP") != string::npos) {
    detID = Const::EDetector::TOP;
    if (name.find("TOPModule") != string::npos) {
      copyID = -touch->GetVolume(0)->GetCopyNo(); // negative to distinguish module and quartz hits
    } else if (name.find("_TOPPrism_") != string::npos or
               name.find("_TOPBarSegment") != string::npos or
               name.find("_TOPMirrorSegment") != string::npos) {
      copyID = (touch->GetHistoryDepth() >= 1) ? touch->GetVolume(1)->GetCopyNo() : 0;
    } else if (name.find("TOPBarSegment1Glue") != string::npos or
               name.find("TOPBarSegment2Glue") != string::npos or
               name.find("TOPMirrorSegmentGlue") != string::npos) {
      copyID = (touch->GetHistoryDepth() >= 2) ? touch->GetVolume(2)->GetCopyNo() : 0;
    }
  }
  // ARICH has an envelope that contains modules that each contain a moduleWindow
  else if (name == "ARICH.AerogelSupportPlate") {
    detID = Const::EDetector::ARICH;
    copyID = 12345;
  } else if (name == "moduleWindow") {
    detID = Const::EDetector::ARICH;
    copyID = (touch->GetHistoryDepth() >= 1) ? touch->GetVolume(1)->GetCopyNo() : 0;
  }
  // ECL crystal (=sensitive) is named "lv_{barrel,forward,backward}_crystal_*"
  else if (name.find("lv_barrel_crystal_") != string::npos ||
           name.find("lv_forward_crystal_") != string::npos ||
           name.find("lv_backward_crystal_") != string::npos) {
    detID = Const::EDetector::ECL;
    copyID = ECL::ECLGeometryPar::Instance()->ECLVolumeToCellID(touch());
  }

}


void TrackExtrapolateG4e::getStartPoint(RecoTrack* recoTrack, const genfit::AbsTrackRep* gfTrackRep, int pdgCode,
                                        G4ThreeVector& position, G4ThreeVector& momentum,
                                        G4ErrorTrajErr& covG4e, double& tof)
{
  try {
    const genfit::MeasuredStateOnPlane& lastState = recoTrack->getMeasuredStateOnPlaneFromLastHit(gfTrackRep);
    TVector3 lastPosition, lastMomentum;
    TMatrixDSym lastCov(6);
    gfTrackRep->getPosMomCov(lastState, lastPosition, lastMomentum, lastCov);

    covG4e = fromPhasespaceToG4e(lastMomentum, lastCov); // in Geant4e units (GeV/c, cm)
    position.setX(lastPosition.X() * CLHEP::cm); // in Geant4 units (mm)
    position.setY(lastPosition.Y() * CLHEP::cm);
    position.setZ(lastPosition.Z() * CLHEP::cm);
    momentum.setX(lastMomentum.X() * CLHEP::GeV);  // in Geant4 units (MeV/c)
    momentum.setY(lastMomentum.Y() * CLHEP::GeV);
    momentum.setZ(lastMomentum.Z() * CLHEP::GeV);

    tof = lastState.getTime(); // NOTE: must be revised, when IP profile (reconstructed beam spot) become available!
    if (pdgCode != lastState.getPDG()) {
      double p2 = lastMomentum.Mag2();
      double mass = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode)->GetPDGMass() / CLHEP::GeV;
      tof *= sqrt((p2 + mass * mass) / (p2 + lastState.getMass() * lastState.getMass()));
    }
  }

  catch (genfit::Exception& e) {
    B2WARNING("Caught genfit exception for last point on track; will not extrapolate. " <<
              e.what());
    // Do not extrapolate this track by forcing minPt cut to fail
    momentum.setX(0.0);
    momentum.setY(0.0);
    momentum.setZ(0.0);
  }
}


TMatrixDSym TrackExtrapolateG4e::fromG4eToPhasespace(const G4ErrorFreeTrajState* g4eState)
{

  // Convert Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in GeV/c, radians, cm)
  // to phase-space covariance matrix with parameters x, y, z, px, py, pz (in GeV/c, cm)
  // (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
  // phi = atan( py / px )
  // lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )
  // xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
  // yT = -x * sin(phi) + y * cos(phi)
  // zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)

  G4ErrorFreeTrajParam param = g4eState->GetParameters();
  double p = 1.0 / (param.GetInvP() * CLHEP::GeV);     // in GeV/c
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

  G4ErrorTrajErr g4eCov = g4eState->GetError();
  G4ErrorSymMatrix phasespaceCov = g4eCov.similarity(jacobian);

  TMatrixDSym covariance(6);
  for (int k = 0; k < 6; k++) {
    for (int j = 0; j < 6; j++) {
      covariance[j][k] = phasespaceCov[j][k];
    }
  }
  return covariance;

}

G4ErrorTrajErr TrackExtrapolateG4e::fromPhasespaceToG4e(const TVector3& momentum, const TMatrixDSym& covariance)
{

  // Convert phase-space covariance matrix with parameters x, y, z, px, py, pz (in genfit2 units: cm, GeV/c)
  // to Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in geant4e units: GeV/c, radians, cm)
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

G4ErrorTrajErr TrackExtrapolateG4e::fromPhasespaceToG4e(const G4ThreeVector& momentum, const G4ErrorSymMatrix& covariance)
{

  // Convert phase-space covariance matrix with parameters x, y, z, px, py, pz (in genfit2 units: cm, GeV/c)
  // to Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in geant4e units: GeV/c, radians, cm)
  // xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
  // yT = -x * sin(phi) + y * cos(phi)
  // zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)
  // (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
  // phi = atan( py / px )
  // lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )

  G4ErrorSymMatrix temp(covariance);

  double sinLambda = momentum.cosTheta();
  double cosLambda = sqrt(1.0 - sinLambda * sinLambda);
  double phi = momentum.phi();
  double cosPhi = cos(phi);
  double sinPhi = sin(phi);
  double pInvSq = 1.0 / momentum.mag2();
  double pInv   = sqrt(pInvSq);
  double pPerpInv = 1.0 / momentum.perp();

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
void TrackExtrapolateG4e::createHit(ExtHitStatus status, ExtState& extState)
{

  StoreArray<ExtHit> extHits(extState.extHitsColName);

  G4StepPoint* stepPoint = extState.g4eState->GetG4Track()->GetStep()->GetPreStepPoint();
  G4TouchableHandle preTouch = stepPoint->GetTouchableHandle();
  G4VPhysicalVolume* preVol = preTouch->GetVolume();

  // Perhaps no hit will be stored?
  if (extState.isMuid) {
    if (find(m_BKLMVolumes->begin(), m_BKLMVolumes->end(), preVol) == m_BKLMVolumes->end() &&
        find(m_EKLMVolumes->begin(), m_EKLMVolumes->end(), preVol) == m_EKLMVolumes->end()) { return; }
  } else {
    if (find(m_EnterExit->begin(), m_EnterExit->end(), preVol) == m_EnterExit->end()) { return; }
  }
  if (status == EXT_EXIT) {
    stepPoint = extState.g4eState->GetG4Track()->GetStep()->GetPostStepPoint();
  }

  TVector3 pos(stepPoint->GetPosition().x() / CLHEP::cm,
               stepPoint->GetPosition().y() / CLHEP::cm,
               stepPoint->GetPosition().z() / CLHEP::cm);
  TVector3 mom(stepPoint->GetMomentum().x() / CLHEP::GeV,
               stepPoint->GetMomentum().y() / CLHEP::GeV,
               stepPoint->GetMomentum().z() / CLHEP::GeV);
  Const::EDetector detID(Const::EDetector::invalidDetector);
  int copyID(0);
  getVolumeID(preTouch, detID, copyID);
  ExtHit* extHit = extHits.appendNew(extState.pdgCode, detID, copyID, status,
                                     extState.tof, pos, mom, fromG4eToPhasespace(extState.g4eState));
  if (extState.track) extState.track->addRelationTo(extHit);

}
