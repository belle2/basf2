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
#include <tracking/dataobjects/ExtRecoHit.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <TMatrixD.h>
#include <TVector3.h>

#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <GFTrack.h>
#include <GFTrackCand.h>
#include <GFDetPlane.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <globals.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4UImanager.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorPropagator.hh>
#include <G4ErrorTrackLengthTarget.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>


using namespace std;
using namespace Belle2;

REG_MODULE(Ext)

ExtModule::ExtModule() : Module(), m_extMgr(NULL)    // no ExtManager yet
{
  setDescription("Extrapolates tracks from CDC to outer detectors using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the reconstructed tracks", string(""));
  addParam("ExtTrackCandsColName", m_extTrackCandsColName, "Name of collection holding the list of hits from each extrapolation", string(""));
  addParam("ExtRecoHitsColName", m_extRecoHitsColName, "Name of collection holding the RecoHits from the extrapolation", string(""));
  addParam("MinPt", m_minPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated.", 0.0);
  addParam("MinKE", m_minKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation.", 0.001);
  addParam("MaxStep", m_maxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity).", 0.0);
  addParam("Cosmic", m_cosmic, "Particle source (0 = beam, 1 = cosmic ray.", 0);
}

ExtModule::~ExtModule()
{
}

void ExtModule::initialize()
{

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
    m_extMgr->SetUserInitialization(new ExtPhysicsList);
  } else {
    // ext will coexist with simulation
    m_runMgr = G4RunManager::GetRunManager();
    m_trk    = const_cast<G4UserTrackingAction*>(m_runMgr->GetUserTrackingAction());
    m_stp    = const_cast<G4UserSteppingAction*>(m_runMgr->GetUserSteppingAction());
  }
  m_extMgr->InitGeant4e();
  G4StateManager::GetStateManager()->SetNewState(G4State_Idle);

  // Redefine step length (cm), magnetic field step limitation (Tesla per GeV/c), and
  // kinetic energy loss limitation (fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  G4double maxStep = std::min(5.0 * cm, m_maxStep);
  if (maxStep > 0.0) {
    char stepSize[80];
    std::sprintf(stepSize, "/geant4e/limits/stepLength %8.2f cm", maxStep);
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

  // PDG codes for the extrapolation hypotheses
  m_pdg[0] = G4ParticleTable::GetParticleTable()->FindParticle("pi+")->GetPDGEncoding();
  m_pdg[1] = G4ParticleTable::GetParticleTable()->FindParticle("e+")->GetPDGEncoding();
  m_pdg[2] = G4ParticleTable::GetParticleTable()->FindParticle("mu+")->GetPDGEncoding();
  m_pdg[3] = G4ParticleTable::GetParticleTable()->FindParticle("kaon+")->GetPDGEncoding();
  m_pdg[4] = G4ParticleTable::GetParticleTable()->FindParticle("proton")->GetPDGEncoding();

  StoreArray<GFTrackCand> dummyCands(m_extTrackCandsColName);
  StoreArray<ExtRecoHit> dummyHits(m_extRecoHitsColName);

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

  StoreArray<GFTrack> gfTracks(m_gfTracksColName);
  StoreArray<GFTrackCand> extTrackCands(m_extTrackCandsColName);
  StoreArray<ExtRecoHit> extRecoHits(m_extRecoHitsColName);
  extTrackCands.getPtr()->Clear();
  extRecoHits.getPtr()->Clear();

  G4ThreeVector position;
  G4ThreeVector momentum;
  G4ErrorTrajErr g4eCov(5, 0);

  int nTracks = gfTracks.getEntries();
  for (int t = 0; t < nTracks; ++t) {

    int charge = int(gfTracks[t]->getCardinalRep()->getCharge());

    for (int hypothesis = 0; hypothesis < N_HYPOTHESES; hypothesis++) {

      GFTrackCand* cand = addTrackCand(gfTracks[t], m_pdg[hypothesis] * charge, extTrackCands, position, momentum, g4eCov);
      if (gfTracks[t]->getMom().Pt() <= m_minPt) continue;
      G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(m_pdg[hypothesis] * charge);
      string g4eName = "g4e_" + particle->GetParticleName();
      double mass = particle->GetPDGMass();
      double minP = sqrt(mass * mass + m_minKE * m_minKE);
      G4ErrorFreeTrajState* state = new G4ErrorFreeTrajState(g4eName, position, momentum, g4eCov);
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
          addFirstPoint(state, cand, extRecoHits);
        }
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (length > 0.0) {
          if (preStatus == fGeomBoundary) {      // first step in this volume?
            addPoint(state, ENTER, cand, extRecoHits);
          }
          // Last step in this volume?
          if (postStatus == fGeomBoundary) {
            addPoint(state, EXIT, cand, extRecoHits);
          }
        }
        // Post-step momentum too low?
        if (errCode || (track->GetMomentum().mag() < minP)) {
          addPoint(state, STOP, cand, extRecoHits);
          break;
        }
        if (m_extMgr->GetPropagator()->CheckIfLastStep(track)) {
          addPoint(state, ESCAPE, cand, extRecoHits);
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
    B2FATAL("Module ext: No geometry defined. Please create the geometry first.")
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

// Convert the physical volume name to an integer pair that identifies it
void ExtModule::getVolumeID(const G4TouchableHandle& touch, int& detID, int& copyID)
{
  G4String name = touch->GetVolume(0)->GetName();
  if (name.find("CDC") != string::npos) {
    detID = 3;
    copyID = touch->GetVolume(0)->GetCopyNo();
  }
  // TOP doesn't have one envelope; it has several "PlacedTOPModule"s
  if (name == "PlacedTOPModule") {
    detID = 3;
    copyID = 0;
  }
  // TOP quartz bar (=sensitive) has an automatically generated PV name
  // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
  // YYY is cuttest.
  if (name.find("_cuttest_") != string::npos) {
    detID = 3;
    copyID = (touch->GetHistoryDepth() >= 2) ? touch->GetVolume(2)->GetCopyNo() : 0;
  }
  // ARICH has an envelope that contains modules that each contain a moduleSensitive
  if (name == "ARICH.Envelope") {
    detID = 4;
    copyID = 0;
  }
  if (name == "ARICH.DetectorModules") {
    detID = 4;
    copyID = 0;
  }
  if (name == "moduleSensitive") {
    detID = 4;
    copyID = (touch->GetHistoryDepth() >= 1) ? touch->GetVolume(1)->GetCopyNo() : 0;
  }
  // ECL
  if (name == "physicalECL") {
    detID = 5;
    copyID = 0;
  }
  // ECL crystal (=sensitive) has an automatically generated PV name
  // av_WWW_impr_XXX_YYY_ZZZ because it is an imprint of a G4AssemblyVolume;
  // YYY is logicalEcl**Crystal, where **=Br, Fw, or Bw.
  // XXX is 1..144 for Br, 1..16 for Fw, and 1..16 for Bw
  // ZZZ is n_pv_m where n is 1..46 for Br, 1..72 for Fw, and 73..132 for Bw
  // CopyNo() encodes XXX and n.
  if (name.find("_logicalEclBrCrystal_") != string::npos) {
    detID = 5;
    copyID = (touch->GetVolume(0)->GetCopyNo() + 1) / 2;
  }
  if (name.find("_logicalEclFwCrystal_") != string::npos) {
    detID = 5;
    copyID = (touch->GetVolume(0)->GetCopyNo() + 1) / 2;
  }
  if (name.find("_logicalEclBwCrystal_") != string::npos) {
    detID = 5;
    copyID = (touch->GetVolume(0)->GetCopyNo() + 1) / 2;
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

  // Transformation Jacobian 6x5 from Geant4e 5x5 to Panther 6x6
  // Jacobian units are GeV/c, radians, cm
  // Geant4e covariance matrix units are GeV/c, radians, cm (!!!) - see PropagateError()
  // Phase-space covariance matrix units are GeV/c, cm

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
  G4ErrorTrajErr phaseSpaceCov = g4eCov.similarity(jacobian);
  TMatrixD cov(6, 6);
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      cov[i][j] = phaseSpaceCov[i][j];
    }
  }
  return cov;

}

GFTrackCand* ExtModule::addTrackCand(const GFTrack* gfTrack, int pdgCode, StoreArray<GFTrackCand>& extTrackCands,
                                     G4ThreeVector& position, G4ThreeVector& momentum, G4ErrorTrajErr& g4eCov)
{

  TMatrixD lastState(gfTrack->getCardinalRep()->getLastState());
  TMatrixD lastCov(gfTrack->getCardinalRep()->getLastCov());
  GFDetPlane lastPlane(gfTrack->getCardinalRep()->getLastPlane());
  double qop = lastState[0][0];
  const TVector3 o = lastPlane.getO();
  const TVector3 u = lastPlane.getU();
  const TVector3 v = lastPlane.getV();
  const TVector3 w = lastPlane.getNormal();
  TVector3 lastPosition = o + lastState[3][0] * u + lastState[4][0] * v;
  TVector3 lastMomentum = w + lastState[1][0] * u + lastState[2][0] * v;
  double spu = 1.0 / (fabs(qop) * lastMomentum.Mag());
  //J_pM matrix is d(x,y,z,px,py,pz) / d(q/p,u',v',u,v)
  TMatrixD J_pM(6, 5);
  // dx/du
  J_pM[0][3] = u.X();
  J_pM[1][3] = u.Y();
  J_pM[2][3] = u.Z();
  // dx/dv
  J_pM[0][4] = v.X();
  J_pM[1][4] = v.Y();
  J_pM[2][4] = v.Z();
  // dpx/dqop
  J_pM[3][0] = -(spu / qop) * lastMomentum.X();
  J_pM[4][0] = -(spu / qop) * lastMomentum.Y();
  J_pM[5][0] = -(spu / qop) * lastMomentum.Z();
  // dpx/du'
  J_pM[3][1] = spu * u.X();
  J_pM[4][1] = spu * u.Y();
  J_pM[5][1] = spu * u.Z();
  // dpx/dv'
  J_pM[3][2] = spu * v.X();
  J_pM[4][2] = spu * v.Y();
  J_pM[5][2] = spu * v.Z();
  TMatrixD J_pM_transp(J_pM);
  J_pM_transp.T();
  TMatrixD out = J_pM * (lastCov * J_pM_transp);
  // The above covariance matrix needs to be reviewed then put into g4eCov.
  TVector3 lastPosError(sqrt(out[0][0]), sqrt(out[1][1]), sqrt(out[2][2]));
  TVector3 lastDirError(sqrt(out[3][3]), sqrt(out[4][4]), sqrt(out[5][5]));
  lastDirError *= fabs(qop);
  int candNumber = extTrackCands->GetLast() + 1;
  GFTrackCand* cand = new(extTrackCands->AddrAt(candNumber)) GFTrackCand();
  cand->setComplTrackSeed(lastPosition, lastMomentum, pdgCode, lastPosError, lastDirError);
  position.setX(lastPosition.x()*cm);
  position.setY(lastPosition.y()*cm);
  position.setZ(lastPosition.z()*cm);
  lastMomentum.SetMag(1.0 / fabs(qop));
  momentum.setX(lastMomentum.x()*GeV);
  momentum.setY(lastMomentum.y()*GeV);
  momentum.setZ(lastMomentum.z()*GeV);

  return cand;
}

void ExtModule::addFirstPoint(const G4ErrorFreeTrajState* state, GFTrackCand* cand, StoreArray<ExtRecoHit>& extRecoHits)
{

  G4StepPoint* stepPoint = state->GetG4Track()->GetStep()->GetPreStepPoint();
  G4TouchableHandle preTouch = stepPoint->GetTouchableHandle();

  // This starting point is typically in the CDC.
  // Write it, even though CDC isn't on the m_enter list
  TMatrixD phasespacePoint(6, 1);
  TMatrixD covariance(6, 6);
  phasespacePoint[0][0] = stepPoint->GetPosition().x() / cm;
  phasespacePoint[1][0] = stepPoint->GetPosition().y() / cm;
  phasespacePoint[2][0] = stepPoint->GetPosition().z() / cm;
  phasespacePoint[3][0] = stepPoint->GetMomentum().x() / GeV;
  phasespacePoint[4][0] = stepPoint->GetMomentum().y() / GeV;
  phasespacePoint[5][0] = stepPoint->GetMomentum().z() / GeV;
  covariance = getCov(state);
  int hitNumber = extRecoHits->GetLast() + 1;
  new(extRecoHits->AddrAt(hitNumber)) ExtRecoHit(phasespacePoint, covariance, ENTER);
  int detID(0);
  int copyID(0);
  getVolumeID(preTouch, detID, copyID);
  cand->addHit(detID, hitNumber, 0.0, copyID);

}

// write another volume-entry or volume-exit point on track
void ExtModule::addPoint(const G4ErrorFreeTrajState* state, ExtHitStatus status, GFTrackCand* cand, StoreArray<ExtRecoHit>& extRecoHits)
{

  G4StepPoint* stepPoint = state->GetG4Track()->GetStep()->GetPreStepPoint();
  G4TouchableHandle preTouch = stepPoint->GetTouchableHandle();

  G4VPhysicalVolume* preVol = preTouch->GetVolume();
  if (status == ENTER) {
    if (find(m_enter->begin(), m_enter->end(), preVol) == m_enter->end()) { return; }
  } else {
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
  int hitNumber = extRecoHits->GetLast() + 1;
  new(extRecoHits->AddrAt(hitNumber)) ExtRecoHit(phasespacePoint, covariance, status);
  int detID(0);
  int copyID(0);
  getVolumeID(preTouch, detID, copyID);
  cand->addHit(detID, hitNumber, 0.0, copyID);

}
