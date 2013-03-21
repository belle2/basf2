/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/muid/MuidModule.h>
#include <tracking/modules/ext/ExtManager.h>
#include <tracking/modules/ext/ExtPhysicsList.h>
#include <tracking/modules/ext/ExtCylSurfaceTarget.h>
#include <tracking/dataobjects/ExtRecoHit.h>
#include <tracking/dataobjects/Muid.h>
#include <tracking/dataobjects/MuidHit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/MagneticField.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <bklm/geometry/GeometryPar.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <TMatrixD.h>
#include <TVector3.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <GFTrack.h>
#include <GFTrackCand.h>
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
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>

using namespace std;
using namespace CLHEP;
using namespace Belle2;

#define TWOPI 6.283185482025146484375
#define M_PI_8 0.3926990926265716552734
#define COS_SMALL_ANGLE 0.98

#define Large 10.0E10

#define MUON 0
#define PION 1
#define KAON 2

REG_MODULE(Muid)

MuidModule::MuidModule() : Module(), m_extMgr(NULL)    // no MuidManager yet
{
  m_pdgCode.clear();
  setDescription("Identifies muons by extrapolating tracks from CDC to KLM using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("pdgCodes", m_pdgCode, "Positive-charge PDG codes for extrapolation hypotheses", m_pdgCode);
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the reconstructed tracks", string(""));
  addParam("MuidsColName", m_muidsColName, "Name of collection holding the muon identification information from the extrapolation", string(""));
  addParam("MuidHitsColName", m_muidHitsColName, "Name of collection holding the muidHits from the extrapolation", string(""));
  addParam("MinPt", m_minPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated.", double(0.1));
  addParam("MinKE", m_minKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation.", double(0.002));
  addParam("MaxStep", m_maxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity).", double(25.0));
  addParam("Cosmic", m_cosmic, "Particle source (0 = beam, 1 = cosmic ray.", 0);
}

MuidModule::~MuidModule()
{
}

void MuidModule::initialize()
{

  // Convert from GeV to GEANT4 energy units (MeV)
  m_minKE = m_minKE * GeV;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Define the geant4e extrapolation Manager.
  m_extMgr = ExtManager::GetManager();

  // See if muid will coexist with geant4 simulation.
  // (The particle list will have been constructed already, if so.)
  if (G4ParticleTable::GetParticleTable()->entries() == 0) {
    // muid will run without simulation
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
  G4double maxStep = min(10.0, m_maxStep) * cm;
  if (maxStep > 0.0) {
    char stepSize[80];
    sprintf(stepSize, "/geant4e/limits/stepLength %8.2f mm", maxStep);
    G4UImanager::GetUIpointer()->ApplyCommand(stepSize);
  }
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  GearDir eklmContent = GearDir("Detector/DetectorComponent[@name=\"EKLM\"]/Content/");
  double eklmLength = eklmContent.getLength("Length") * cm;
  GearDir bklmContent = GearDir("Detector/DetectorComponent[@name=\"BKLM\"]/Content/");
  double bklmHalfLength = bklmContent.getLength("HalfLength") * cm;
  double offsetZ = bklmContent.getLength("OffsetZ") * cm;
  double minZ = offsetZ - (bklmHalfLength + eklmLength);
  double maxZ = offsetZ + (bklmHalfLength + eklmLength);
  double outerRadius = bklmContent.getLength("OuterRadius") * cm;
  double nSector = bklmContent.getNumberNodes("Sectors/Forward/Sector");
  double rMax = outerRadius / cos(M_PI / nSector);
  //std::cout << "MUID initialize: eklmLength=" << eklmLength << "  bklmHalfLength=" << bklmHalfLength << "  offsetZ=" << offsetZ << "  minZ=" << minZ << "  maxZ=" << maxZ << "  outerRadius=" << outerRadius << "  nSector=" << nSector << "  rMax=" << rMax << std::endl;
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(new ExtCylSurfaceTarget(rMax, minZ, maxZ));
  m_OffsetZ = bklmContent.getLength("OffsetZ");
  m_EndcapMaxR = eklmContent.getLength("EndCap/OuterR"); // 290.0 cm --> 332.0 cm
  m_EndcapMinR = eklmContent.getLength("EndCap/InnerR"); // 125.0 cm --> 130.5 cm
  m_BarrelMinR = bklmContent.getLength("Layers/InnerRadius"); // 202.0 cm --> 201.9 cm ok
  m_EndcapHalfLength = eklmContent.getLength("EndCap/Length") * 0.5; // 69.5 cm --> 66.05 cm ok
  m_BarrelHalfLength = bklmContent.getLength("Layers/GapLength");  // 222.0 cm ok
  m_EndcapLayers = eklmContent.getInt("EndCap/nLayer"); // 14 ok
  m_BarrelLayers = bklmContent.getNumberNodes("Layers/Layer"); // 15 ok
  m_EndcapMiddleZ = eklmContent.getLength("HalfLength") + m_EndcapHalfLength; // 291.5 cm --> 296.05 cm ok
  m_StripPositionError = 4.0; // 4.0 cm DIVOT should this be 4/sqrt{12}?
  m_DefaultError = 15.0; // 15.0 cm DIVOT
  m_maxDistCM = 25.0; // max distance between KLM hit and extrapolation crossing
  m_maxDistSIGMA = 5.0; // ditto (in sigmas)

  // KLM geometry (for associating KLM hit with extrapolated crossing point) (cm)

  m_BarrelActiveMinZ = bklmContent.getLength("Module/ElectrodeBorder");
  m_BarrelActiveMaxZ = bklmContent.getLength("Layers/Layer[@layer=\"1\"]/PhiStrips/Length")
                       + m_BarrelActiveMinZ; // was 216.9+2.78 cm --> 216.9+1.0 cm
  for (int layer = 1; layer <= m_BarrelLayers; ++layer) {
    m_BarrelModuleMiddleRadius[layer] = bklm::GeometryPar::instance()->getModuleMiddleRadius(layer); // cm
  }
  double dz(eklmContent.getLength("EndCap/ShiftZ"));
  double z0(eklmContent.getLength("EndCap/PositionZ") - m_OffsetZ + dz - eklmContent.getLength("EndCap/Length") * 0.5);
  for (int layer = 1; layer <= m_EndcapLayers; ++layer) {
    m_EndcapModuleMiddleZ[layer] = z0 + dz * (layer - 1);
  }
  m_EndcapActiveMinR = eklmContent.getLength("Plane/InnerR"); // 133.5 cm --> 132.5 cm
  m_EndcapActiveMaxR = eklmContent.getLength("Plane/OuterR"); // 315.9 cm --> 329.0 cm
  for (int sector = 1; sector <= 8; ++sector) {
    double phi = M_PI_4 * (sector - 1);
    m_BarrelSectorPerp[sector].SetX(cos(phi));
    m_BarrelSectorPerp[sector].SetY(sin(phi));
    m_BarrelSectorPerp[sector].SetZ(0.0);
    m_BarrelSectorPhi[sector].SetX(-sin(phi));
    m_BarrelSectorPhi[sector].SetY(cos(phi));
    m_BarrelSectorPhi[sector].SetZ(0.0);
  }

  // Default hypothesis for extrapolation
  if (m_pdgCode.size() == 0) {
    m_pdgCode.push_back(G4ParticleTable::GetParticleTable()->FindParticle("mu+")->GetPDGEncoding());
  }
  // check that the (user-)defined PDG codes for the extrapolation hypotheses are legal
  for (unsigned int hypothesis = 0; hypothesis < m_pdgCode.size(); ++hypothesis) {
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(m_pdgCode[hypothesis]);
    if (particle->GetPDGCharge() == 0) {
      B2FATAL("Module muid initialize(): Particle " << particle->GetParticleName() << " is neutral. Only charged particles can be extrapolated")
    }
    if (particle->GetPDGCharge() < 0) {
      m_pdgCode[hypothesis] = -m_pdgCode[hypothesis];
      particle = G4ParticleTable::GetParticleTable()->FindParticle(m_pdgCode[hypothesis]);
    }
    string g4eName = "g4e_" + particle->GetParticleName();
    G4ParticleDefinition* g4eParticle = G4ParticleTable::GetParticleTable()->FindParticle(g4eName);
    if (g4eParticle == NULL) {
      B2FATAL("Module muid initialize(): Particle " << g4eName << " is not defined in G4ParticleTable")
    }
    if (G4ParticleTable::GetParticleTable()->FindAntiParticle(g4eName) == NULL) {
      B2FATAL("Module muid initialize(): Antiparticle of " << g4eName << " is not defined in G4ParticleTable")
    }
    B2INFO("Module muid initialize(): hypotheses for " << particle->GetParticleName() << " and its antiparticle will be extrapolated")
  }

  fillPDF(MUON);
  fillPDF(PION);
  fillPDF(KAON);

  StoreArray<GFTrack> gfTracks(m_gfTracksColName);
  StoreArray<Muid> muids(m_muidsColName);
  RelationArray gfTrackToMuid(gfTracks, muids);
  StoreArray<MuidHit> muidHits(m_muidHitsColName);
  RelationArray gfTrackToMuidHits(gfTracks, muidHits);
  StoreArray<Muid>::registerPersistent();
  StoreArray<MuidHit>::registerPersistent();
  RelationArray::registerPersistent<GFTrack, Muid>();
  RelationArray::registerPersistent<GFTrack, MuidHit>();

  return;

}

void MuidModule::beginRun()
{
}

void MuidModule::event()
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

  StoreArray<BKLMHit2d> bklmHits(m_bklmHitsColName);
  StoreArray<EKLMHit2d> eklmHits(m_eklmHitsColName);
  StoreArray<GFTrack> gfTracks(m_gfTracksColName);
  StoreArray<Muid> muids(m_muidsColName);
  RelationArray gfTrackToMuid(gfTracks, muids);
  StoreArray<MuidHit> muidHits(m_muidHitsColName);
  RelationArray gfTrackToMuidHits(gfTracks, muidHits);
  muids.getPtr()->Clear();
  muidHits.getPtr()->Clear();

  G4ThreeVector position;
  G4ThreeVector momentum;
  G4ErrorTrajErr covG4e(5, 0);

  int nTracks = gfTracks.getEntries();
  for (int t = 0; t < nTracks; ++t) {

    int charge = int(gfTracks[t]->getCardinalRep()->getCharge());

    for (unsigned int hypothesis = 0; hypothesis < m_pdgCode.size(); hypothesis++) {

      int pdgCode = m_pdgCode[hypothesis] * charge;
      Muid* muid = new(muids.nextFreeAddress()) Muid(pdgCode);
      gfTrackToMuid.add(t, muids.getEntries() - 1);
      getStartPoint(gfTracks[t], pdgCode, position, momentum, covG4e);
      if (gfTracks[t]->getMom().Pt() <= m_minPt) continue;
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
        //const G4int    postStatus = step->GetPostStepPoint()->GetStepStatus();
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (length > 0.0) {
          if (preStatus == fGeomBoundary) {      // first step in this volume?
            if (createHit(state, t, pdgCode, muidHits, gfTrackToMuidHits, bklmHits, eklmHits)) {
              break;
            }
          }
          m_tof += step->GetDeltaTime();
        }
        // Post-step momentum too low?
        if (errCode || (track->GetMomentum().mag() < minP)) {
          break;
        }
        // Reached the target boundary?
        if (G4ErrorPropagatorData::GetErrorPropagatorData()->GetState() == G4ErrorState(G4ErrorState_TargetCloserThanBoundary)) {
          break;
        }
        if (m_extMgr->GetPropagator()->CheckIfLastStep(track)) {
          break;
        }

      } // track-extrapolation "infinite" loop

      m_extMgr->EventTermination();

      delete state;

      finishTrack(muid);

    } // hypothesis loop

  } // track loop

  if (m_runMgr) {
    m_runMgr->SetUserAction(m_trk);
    m_runMgr->SetUserAction(m_stp);
  }

}

void MuidModule::endRun()
{
}

void MuidModule::terminate()
{

  if (m_runMgr) {
    m_runMgr->SetUserAction(m_trk);
    m_runMgr->SetUserAction(m_stp);
  }
  delete m_enter;
  delete m_exit;

}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void MuidModule::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("Module muid registerVolumes(): No geometry defined. Please create the geometry first.")
  }

  m_enter = new vector<G4VPhysicalVolume*>;
  m_exit  = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // see belle2/run/volname3.txt:
    // Barrel KLM: BKLM.EnvelopePhysical and BKLM.Gas_*_*_**_*
    if ((name == "BKLM.EnvelopePhysical") || (name.substr(0, 8) == "BKLM.Gas") || (name.substr(0, 8) == "BKLM.Sci")) {
      m_enter->push_back(*iVol);
      m_exit->push_back(*iVol);
    }
    // Endcap KLM: Endcap_{1,2} and
    // Sensitive_Strip_StripVolume_{1..75}_Plane_{1,2}_Sector_{1..4}_Layer_{1..14}_Endcap_{1,2}
    if ((name == "Endcap_1") || (name == "Endcap_2") || (name.substr(0, 27) == "Sensitive_Strip_StripVolume")) {
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
void MuidModule::getVolumeID(const G4TouchableHandle& touch, ExtDetectorID& detID, int& copyID)
{

  // default values
  detID = EXT_UNKNOWN;
  copyID = 0;

  G4String name = touch->GetVolume(0)->GetName();
  if (name.find("CDC") != string::npos) {
    detID = EXT_CDC;
    copyID = touch->GetVolume(0)->GetCopyNo();
  }
  // Barrel KLM: BKLM.EnvelopePhysical and BKLM.Gas_*_*_**_* and BKLM.Sci_*_*_**_*
  if (name == "BKLM.EnvelopePhysical") {
    detID = EXT_BKLM;
  }
  if ((name.substr(0, 8) == "BKLM.Gas") || (name.substr(0, 8) == "BKLM.Sci")) {
    detID = EXT_BKLM;
    int forward = (name.substr(9, 1) == "F") ? 1 : 2;
    int sector = boost::lexical_cast<int>(name.substr(11, 1));
    int layer = boost::lexical_cast<int>(name.substr(13, 2));
    int plane = (name.substr(16, 5) == "Inner") ? PLANE_INNER : PLANE_OUTER;
    copyID = (((((forward << 8) + sector) << 8) + layer) << 8) + plane;
  }
  // Endcap KLM: Endcap_{1,2} and
  // Sensitive_Strip_StripVolume_{1..75}_Plane_{1,2}_Sector_{1..4}_Layer_{1..14}_Endcap_{1,2}
  // Sensitive_Strip_StripVolume_{1..75}_Plane_{1,2}_Sector_{1..4}_Layer_{1..14}_Endcap_{1,2}
  if ((name == "Endcap_1") || (name == "Endcap_2")) {
    detID = EXT_EKLM;
  }
  if (name.substr(0, 27) == "Sensitive_Strip_StripVolume") {
    detID = EXT_EKLM;
    int i0 = name.size() - 63;
    if ((i0 == 0) || (i0 == 1)) {
      int forward = boost::lexical_cast<int>(name.substr(i0 + 62, 1));
      int sector = boost::lexical_cast<int>(name.substr(i0 + 45, 1));
      int layer = boost::lexical_cast<int>(name.substr(i0 + 53, 1));
      int plane = boost::lexical_cast<int>(name.substr(i0 + 36, 1));
      copyID = (((((forward << 8) + sector) << 8) + layer) << 8) + plane;
    }
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

TMatrixD MuidModule::getCov(const G4ErrorFreeTrajState* state)
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

void MuidModule::getStartPoint(const GFTrack* gfTrack, int pdgCode,
                               G4ThreeVector& position, G4ThreeVector& momentum, G4ErrorTrajErr& covG4e)
{

  GFAbsTrackRep* gfTrackRep = gfTrack->getCardinalRep();
  for (unsigned int rep = 0; rep < gfTrack->getNumReps(); ++rep) {
    if (gfTrack->getTrackRep(rep)->getPDG() == pdgCode) {
      gfTrackRep = gfTrack->getTrackRep(rep);
      break;
    }
  }
  TVectorD firstState = gfTrackRep->getFirstState();
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
  const TVectorD lastState(gfTrackRep->getLastState());
  TMatrixD lastCov(gfTrackRep->getLastCov());
  GFDetPlane lastPlane(gfTrackRep->getLastPlane());
  const TVector3 lastO = lastPlane.getO();
  const TVector3 lastU = lastPlane.getU();
  const TVector3 lastV = lastPlane.getV();
  const TVector3 lastW = lastPlane.getNormal();
  const TVector3 lastPosition = lastO + lastState[3] * lastU + lastState[4] * lastV;
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
  const TVector3 lastMomentum = lastMomMag * lastDirection;
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
  const TMatrixD lastCovPS = fromGenfitToPhasespace * (lastCov * fromGenfitToPhasespaceT);
  const TMatrixDSym lastCovPSSym = TMatrixDSym(6, lastCovPS.GetMatrixArray());
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

  // Keep track of geometrical state during one track's extrapolation
  m_leftKLM = false;
  m_fromBarrelToEndcap = false;
  m_sideGapEscape = 0;
  m_wasInBarrelIron = true;
  m_wasInBarrel = false;
  m_wasInEndcap = false;
  m_geantBarrelLayer = -1;        // g4e hasn't entered a barrel sensitive volume yet
  m_geantEndcapLayer = -1;        // g4e hasn't entered an endcap sensitive volume yet
  m_firstBarrelLayer = 0;         // ratchet outward when looking for matching barrel hits
  m_firstEndcapLayer = 0;         // ratchet outward when looking for matching endcap hits
  m_address.isRPC = true;         // prior crossed layer is invalid initially
  m_address.inBarrel = false;     // ditto
  m_address.isForward = false;    // ditto
  m_address.sector = -1;          // ditto
  m_address.layer = -1;           // ditto
  m_address.isInnerPlane = false; // ditto
  m_position = TVector3(0.0, 0.0, 0.0);

  // Quantities that will be written to StoreArray<Muid> at end of track
  m_enteredPattern = 0x00000000;
  m_matchedPattern = 0x00000000;
  m_chi2 = 0.0;
  m_nPoint = 0;
  m_lastBarrelLayerExt = -1;    // track hasn't crossed a barrel layer yet
  m_lastBarrelLayerHit = -1;    // track hasn't crossed an endcap layer yet
  m_lastEndcapLayerExt = -1;    // no matching hit in barrel layer yet
  m_lastEndcapLayerHit = -1;    // no matching hit in endcap layer yet
  m_numBarrelLayerExt = 0;      // ditto
  m_numBarrelLayerHit = 0;      // ditto
  m_numEndcapLayerExt = 0;      // ditto
  m_numEndcapLayerHit = 0;      // ditto

  return;

}

// Write another volume-entry point on track.
// The track state will be modified here by the Kalman fitter.
bool MuidModule::createHit(G4ErrorFreeTrajState* state, int trackID, int pdgCode,
                           StoreArray<MuidHit>& muidHits, RelationArray& gfTrackToMuidHits,
                           StoreArray<BKLMHit2d>& bklmHits, StoreArray<EKLMHit2d>& eklmHits)
{

  G4StepPoint* stepPoint = state->GetG4Track()->GetStep()->GetPreStepPoint();
  G4TouchableHandle preTouch = stepPoint->GetTouchableHandle();
  G4VPhysicalVolume* preVol = preTouch->GetVolume();
  TVector3 position(stepPoint->GetPosition().x() / cm, stepPoint->GetPosition().y() / cm, stepPoint->GetPosition().z() / cm);
  double r = position.Perp();
  double z = fabs(position.z() - m_OffsetZ);

  TMatrixD phasespacePoint(6, 1);
  TMatrixD covariance(6, 6);
  phasespacePoint[0][0] = stepPoint->GetPosition().x() / cm;
  phasespacePoint[1][0] = stepPoint->GetPosition().y() / cm;
  phasespacePoint[2][0] = stepPoint->GetPosition().z() / cm;
  phasespacePoint[3][0] = stepPoint->GetMomentum().x() / GeV;
  phasespacePoint[4][0] = stepPoint->GetMomentum().y() / GeV;
  phasespacePoint[5][0] = stepPoint->GetMomentum().z() / GeV;
  covariance = getCov(state);

  bool isInEndcap((r > m_EndcapMinR) && (fabs(z - m_EndcapMiddleZ) < m_EndcapHalfLength));
  bool isInBarrel((r > m_BarrelMinR) && (z < m_BarrelHalfLength));

  if (m_wasInBarrel && isInEndcap) {
    m_fromBarrelToEndcap = true;
    if (!m_wasInBarrelIron) {
      m_sideGapEscape = 1;
    }
  }

  ExtDetectorID detID = EXT_UNKNOWN;
  int copyID = 0;
  getVolumeID(preTouch, detID, copyID);
  m_wasInBarrelIron = (detID == EXT_BKLM);

  if (m_wasInEndcap || m_wasInBarrel || isInEndcap || isInBarrel) {

    Point p;
    p.enteredSensitiveVolume = false;
    p.intersected = false;
    p.hasMatchingHit = false;
    p.covariance = HepSymMatrix(6, 0);

    vector<G4VPhysicalVolume*>::iterator j;
    for (j = m_enter->begin(); (j != m_enter->end()) && (*j != preVol); j++) {}
    if (j != m_enter->end()) {        // entered a KLM sensitive volume?

      p.enteredSensitiveVolume = true;
      getAddress(preVol->GetName(), p.address);
      if (p.address.inBarrel) {
        if (m_geantBarrelLayer < p.address.layer) {
          if ((m_geantBarrelLayer < 14) || (z < m_BarrelHalfLength - 10.0)) {
            m_geantBarrelLayer = p.address.layer;
            if (m_geantBarrelLayer > m_lastBarrelLayerExt) {
              m_numBarrelLayerExt++;
              m_lastBarrelLayerExt = m_geantBarrelLayer;
            }
          }
        }
      } else {
        if (p.address.layer <= m_EndcapMaxLayer) {
          if (r < m_EndcapMaxR) {
            m_geantEndcapLayer = p.address.layer;
            if (m_geantEndcapLayer > m_lastEndcapLayerExt) {
              m_numEndcapLayerExt++;
              m_lastEndcapLayerExt = m_geantEndcapLayer;
            }
          }
        }
        if (m_geantEndcapLayer > m_EndcapMaxLayer) { return true; }
      }

    }

    // Did the track cross a KLM detector layer?

    p.momentum.SetX(stepPoint->GetMomentum().x() / GeV);
    p.momentum.SetY(stepPoint->GetMomentum().y() / GeV);
    p.momentum.SetZ(stepPoint->GetMomentum().z() / GeV);
    fromG4eToPhasespace(state, p.covariance);

    bool searchEndcap(false);
    bool crossed(false);

    if (m_wasInBarrel || isInBarrel) {
      findBarrelIntersection(m_position, position, p);
      if ((p.intersected != MISSED) &&
          ((p.address.inBarrel != m_address.inBarrel) || (p.address.isForward != m_address.isForward) || (p.address.layer != m_address.layer))) {
        crossed = true;
        findMatchingBarrelHit(p, bklmHits);
        if (p.hasMatchingHit) {
          double extTime = 0.0;  // DIVOT should be extrapolated-track time from IP to BKLM
          double hitTime = 0.0;  // DIVOT should be measured hit time
          new(muidHits.nextFreeAddress())
          MuidHit(pdgCode, p.address.inBarrel, p.address.isForward, p.address.sector, p.address.layer, p.position, p.positionAtHitPlane, extTime, hitTime, p.chi2);
          gfTrackToMuidHits.add(trackID, muidHits.getEntries() - 1);
          p.intersected = CROSSED;
          p.enteredSensitiveVolume = true;
          int layer = max(m_geantBarrelLayer, p.address.layer);
          if (layer > m_lastBarrelLayerHit) {
            m_numBarrelLayerHit++;
            m_lastBarrelLayerHit = layer;
          }
        } else if (p.intersected != CROSSED) {
          searchEndcap = true;
        }
      } else if (p.intersected != CROSSED) {
        searchEndcap = true;
      }
    } else {
      searchEndcap = true;
    }

    if (searchEndcap) {

      findEndcapIntersection(m_position, position, p);

      if ((p.intersected != MISSED) &&
          ((p.address.inBarrel != m_address.inBarrel) || (p.address.isForward != m_address.isForward) || (p.address.layer != m_address.layer))) {
        crossed = true;
        findMatchingEndcapHit(p, eklmHits);
        if (p.hasMatchingHit) {
          double extTime = 0.0;  // DIVOT should be extrapolated-track time from IP to EKLM
          double hitTime = 0.0;  // DIVOT should be measured hit time
          new(muidHits.nextFreeAddress())
          MuidHit(pdgCode, p.address.inBarrel, p.address.isForward, p.address.sector, p.address.layer, p.position, p.positionAtHitPlane, extTime, hitTime, p.chi2);
          gfTrackToMuidHits.add(trackID, muidHits.getEntries() - 1);
          p.intersected = CROSSED;
          p.enteredSensitiveVolume = true;
          int layer = max(m_geantEndcapLayer, p.address.layer);
          if (layer > m_EndcapMaxLayer) { return true; }
          if (layer > m_lastEndcapLayerHit) {
            m_numEndcapLayerHit++;
            m_lastEndcapLayerHit = layer;
          }
        }
      }
    }

    if (crossed || p.hasMatchingHit) {
      if (p.hasMatchingHit) {
        G4Point3D newPos;
        newPos.set(p.position.x()*cm, p.position.y()*cm, p.position.z()*cm);
        state->SetPosition(newPos);
        G4Vector3D newMom;
        newMom.set(p.momentum.X()*GeV, p.momentum.Y()*GeV, p.momentum.Z()*GeV);
        state->SetMomentum(newMom);
        // DIVOT this doesn't work??
        G4ErrorTrajErr g4eCov(5, 0);
        fromPhasespaceToG4e(p, g4eCov);
        state->SetError(g4eCov);
        position = p.position;
      }
      m_address = p.address;
    }

    m_numBarrelLayerExt = max(m_numBarrelLayerExt, m_numBarrelLayerHit);
    m_numEndcapLayerExt = max(m_numEndcapLayerExt, m_numEndcapLayerHit);
    m_lastBarrelLayerExt = max(m_lastBarrelLayerExt, m_lastBarrelLayerHit);
    m_lastEndcapLayerExt = max(m_lastEndcapLayerExt, m_lastEndcapLayerHit);

    if ((!p.hasMatchingHit) && (m_sideGapEscape == 1)) {
      // DIVOT Is this ad hoc adjustment for some obscure feature of the geant3 simulation geometry?
      p.momentum += p.momentum.Unit() * 50.0 * MeV;
      G4Vector3D newMom(p.momentum.X(), p.momentum.Y(), p.momentum.Z());
      state->SetMomentum(newMom * GeV);
      m_sideGapEscape = 2;
    }

    if (p.address.inBarrel) {
      if (p.enteredSensitiveVolume) {
        m_enteredPattern |= (0x00000001 << p.address.layer);
      }
      if (p.hasMatchingHit) {
        m_matchedPattern |= (0x00000001 << p.address.layer);
      }
    } else {
      if (p.enteredSensitiveVolume) {
        m_enteredPattern |= (0x00008000 << p.address.layer);
      }
      if (p.hasMatchingHit) {
        m_matchedPattern |= (0x00008000 << p.address.layer);
      }
    }

  } // if (m_wasInEndcap || m_wasInBarrel || isInEndcap || isInBarrel)

  m_position = position;
  m_wasInBarrel = isInBarrel;
  m_wasInEndcap = isInEndcap;

  return false;

}

void MuidModule::findBarrelIntersection(TVector3& oldPos, TVector3& newPos, Point& p)
{

  int sector[2];
  double phi = oldPos.Phi();
  if (phi < 0.0) { phi += TWOPI; }
  if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
  sector[0] = (int)((phi + M_PI_8) / M_PI_4);

  phi = newPos.Phi();
  if (phi < 0.0) { phi += TWOPI; }
  if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
  sector[1] = (int)((phi + M_PI_8) / M_PI_4);

  p.intersected = MISSED;
  for (int j = 0; j < 2; j++) {
    if ((j == 0) && (sector[0] == sector[1])) { continue; }
    double oldR = oldPos * m_BarrelSectorPerp[sector[j]];
    double newR = newPos * m_BarrelSectorPerp[sector[j]];
    double diffR = newR - oldR;
    for (int layer = m_firstBarrelLayer; layer < m_BarrelLayers; layer++) {
      if (newR <  m_BarrelModuleMiddleRadius[layer]) { break; }
      if (oldR <= m_BarrelModuleMiddleRadius[layer]) {
        if (fabs(diffR) > 1.0E-12) {
          p.position = oldPos + (newPos - oldPos) * ((m_BarrelModuleMiddleRadius[layer] - oldR) / diffR);
          phi = p.position.Phi();
          if (phi < 0.0) { phi += TWOPI; }
          if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
          if (sector[j] == (int)((phi + M_PI_8) / M_PI_4)) {
            m_firstBarrelLayer = layer + 1;
            p.address.layer = layer;
            p.address.sector = sector[j];
            p.address.inBarrel = true;
            p.address.isForward = newPos.z() > m_OffsetZ;
            p.intersected = CROSSED;
          }
        }
        break;
      }
    }
    if (p.intersected == CROSSED) { break; }

  }

  // DIVOT should also check against m_BarrelActiveMinZ
  if (fabs(newPos.z() - m_OffsetZ) > m_BarrelActiveMaxZ) {
    double errTrack = getPlaneError(p.covariance, (newPos - oldPos).Unit(), TVector3(0.0, 0.0, 1.0));
    errTrack = sqrt(errTrack * errTrack + m_StripPositionError * m_StripPositionError);
    errTrack = max(errTrack * m_maxDistSIGMA, m_maxDistCM);
    if (fabs(oldPos.z() - m_OffsetZ) <= m_BarrelActiveMaxZ + errTrack) {
      if ((fabs(p.position.z() - m_OffsetZ) > m_BarrelActiveMaxZ) && (p.intersected == CROSSED)) {
        p.intersected = SIDE;
      }
    } else {
      p.intersected = MISSED;
    }
  }

}

void MuidModule::findEndcapIntersection(TVector3& oldPos, TVector3& newPos, Point& p)
{

  double oldZ = fabs(oldPos.Z() - m_OffsetZ);
  double newZ = fabs(newPos.Z() - m_OffsetZ);
  double diffZ = newZ - oldZ;

  p.intersected = MISSED;
  for (int layer = m_firstEndcapLayer; layer < m_EndcapLayers; layer++) {
    if (newZ <  m_EndcapModuleMiddleZ[layer]) { break; }
    if (oldZ <= m_EndcapModuleMiddleZ[layer]) {
      if (fabs(diffZ) > 1.0E-12) {
        p.position = oldPos + (newPos - oldPos) * ((m_EndcapModuleMiddleZ[layer] - oldZ) / diffZ);
        double radius = p.position.Perp();
        if ((radius >= m_EndcapActiveMinR) && (radius <= m_EndcapActiveMaxR)) {
          p.intersected = CROSSED;
        } else {
          p.intersected = SIDE;
        }
        m_firstEndcapLayer = layer + 1;
        p.address.layer = layer;
        p.address.inBarrel = false;
        p.address.isForward = newPos.Z() > m_OffsetZ;
      }
      break;
    }
  }

  if (oldZ < m_EndcapModuleMiddleZ[1]) {
    double phi = newPos.Phi();
    double errTrack = getPlaneError(p.covariance, (newPos - oldPos).Unit(), TVector3(cos(phi), sin(phi), 0.0));
    errTrack = sqrt(errTrack * errTrack + m_StripPositionError * m_StripPositionError);
    errTrack = max(errTrack * m_maxDistSIGMA, m_maxDistCM);
    if (newZ < m_EndcapModuleMiddleZ[1] - errTrack) {
      p.intersected = MISSED;
    }
  }

}

void MuidModule::findMatchingBarrelHit(Point& p, StoreArray<BKLMHit2d>& bklmHits)
{

  double diffBest = 1.0E12;
  double localPosition[2] = {0.0, 0.0};
  double localError[2] = {0.0, 0.0};

  p.hasMatchingHit = false;
  BKLMHit2d* hitBest = NULL;
  for (int h = 0; h < bklmHits.getEntries(); ++h) {
    BKLMHit2d* hit = bklmHits[h];
    if (hit->getLayer() != p.address.layer) { continue; }
    if (hit->getStatus() & STATUS_OUTOFTIME) { continue; }
    TVector3 diff = hit->getGlobalPosition() - p.position;
    TVector3 diffDir = diff.Unit();
    double diffMag = diff.Mag();
    double errTrack = getPlaneError(p.covariance, p.momentum.Unit(), diffDir);
    double projectionPhi = m_BarrelSectorPhi[hit->getSector()] * diffDir;
    double projectionTheta = diffDir.z();
    hit->getLocalPosition(localPosition, localError);
    double varHit = max(projectionTheta * projectionTheta * localError[1] +
                        projectionPhi * projectionPhi * localError[0], 0.0);
    if (diffMag <= max(m_maxDistSIGMA * sqrt(errTrack * errTrack + varHit), m_maxDistCM)) {
      p.hasMatchingHit = true;
      p.address.isForward = hit->isForward();
      p.address.sector = hit->getSector();
      if (diffMag < diffBest) {
        diffBest = diffMag;
        hitBest = hit;
      }
    }

  }     // for (h)
  if (p.hasMatchingHit) {
    hitBest->getLocalPosition(localPosition, localError);
    TVector3 hitPos = hitBest->getGlobalPosition();
    adjustIntersection(p, localError, hitPos);
    if (p.chi2 >= 0.0) {
      m_chi2 += p.chi2;
      m_nPoint++;
    }
    hitBest->setStatus(STATUS_ONTRACK);
  }

}

void MuidModule::findMatchingEndcapHit(Point& p, StoreArray<EKLMHit2d>& eklmHits)
{

  double diffBest = 1.0E12;
  double localPosition[2] = {0.0, 0.0};
  double localError[2] = {0.0, 0.0};

  p.hasMatchingHit = false;
  EKLMHit2d* hitBest = NULL;
  for (int h = 0; h < eklmHits.getEntries(); ++h) {
    EKLMHit2d* hit = eklmHits[h];
    if (hit->getLayer() != p.address.layer) { continue; }
    // DIVOT no status if (hit->getStatus() & STATUS_OUTOFTIME) { continue; }
    TVector3 diff = hit->getPosition() - p.position;
    TVector3 diffDir = diff.Unit();
    double diffMag = diff.Mag();
    double errTrack = getPlaneError(p.covariance, p.momentum.Unit(), diffDir);
    double projectionPhi = m_BarrelSectorPhi[hit->getSector()] * diffDir;
    double projectionTheta = diffDir.z();
    // DIVOT no localPosition hit->getLocalPosition(localPosition, localError);
    double varHit = max(projectionTheta * projectionTheta * localError[1] +
                        projectionPhi * projectionPhi * localError[0], 0.0);
    if (diffMag <= max(m_maxDistSIGMA * sqrt(errTrack * errTrack + varHit), m_maxDistCM)) {
      p.hasMatchingHit = true;
      p.address.isForward = (hit->getEndcap() == 1);
      p.address.sector = hit->getSector();
      if (diffMag < diffBest) {
        diffBest = diffMag;
        hitBest = hit;
      }
    }

  }     // for (h)
  if (p.hasMatchingHit) {
    // DIVOT no localPosition hitBest->getLocalPosition(localPosition, localError);
    TVector3 hitPos = hitBest->getPosition();
    adjustIntersection(p, localError, hitPos);
    if (p.chi2 >= 0.0) {
      m_chi2 += p.chi2;
      m_nPoint++;
    }
    // DIVOT no setStatus hitBest->setStatus(STATUS_ONTRACK);
  }
}

void MuidModule::adjustIntersection(Point& p, double localError[2], TVector3& hitPos)
{

  p.chi2 = -1.0;

  TVector3 extPos = p.position;
  TVector3 extMom = p.momentum;
  TVector3 extDir = p.momentum.Unit();
  HepSymMatrix extCov = p.covariance;
  TVector3 diffPos = hitPos - extPos;

  TVector3 nPerp;      // unit vector normal to the readout plane
  TVector3 nTheta;     // unit vector along theta readout direction
  TVector3 nPhi;       // unit vector along phi readout direction
  if (p.address.inBarrel) {
    nPerp = m_BarrelSectorPerp[p.address.sector];
    nTheta = TVector3(0.0, 0.0, 1.0);
    nPhi = m_BarrelSectorPhi[p.address.sector];
  } else {
    nPerp = TVector3(0.0, 0.0, (p.address.isForward ? 1.0 : -1.0));
    double phi = extPos.Phi();
    nTheta.SetX(cos(phi));
    nTheta.SetY(sin(phi));
    nTheta.SetZ(0.0);
    nPhi.SetX(-sin(phi));
    nPhi.SetY(cos(phi));
    nPhi.SetZ(0.0);
  }

// Don't adjust the track extrapolation if the track is nearly tangent to the
// readout plane and along one or the other of the readout strips.
// Otherwise, construct the "track coordinate system" basis vectors:
//   nzTCS is the same as extDir, by definition
//   nxTCS is in the plane defined by extDir and nPhi
//   nyTCS is nzTCS x nxTCS (right-handed frame)

  if (fabs(nTheta * extDir) > COS_SMALL_ANGLE) { return; }
  if (fabs(nPhi   * extDir) > COS_SMALL_ANGLE) { return; }
  TVector3 nxTCS(nTheta.Cross(extDir).Unit());
  TVector3 nyTCS(extDir.Cross(nxTCS));

// Setup the projection matrix: projection on the y-x plane of TCS.

  HepMatrix projectToTCS(2, 6, 0);
  projectToTCS(1, 1) = nyTCS.X();
  projectToTCS(1, 2) = nyTCS.Y();
  projectToTCS(1, 3) = nyTCS.Z();
  projectToTCS(2, 1) = nxTCS.X();
  projectToTCS(2, 2) = nxTCS.Y();
  projectToTCS(2, 3) = nxTCS.Z();

// Move the extrapolated coordinate to the plane of the hit RPC.

  TVector3 move = extDir * ((diffPos * nPerp) / (extDir * nPerp));
  extPos += move;
  diffPos -= move;
  p.positionAtHitPlane.SetX(extPos.X());
  p.positionAtHitPlane.SetY(extPos.Y());
  p.positionAtHitPlane.SetZ(extPos.Z());

// Calculate the residuals of EXT track and KLM hit on the y-x plane of TCS.

  double nTheta_nxTCS(nTheta * nxTCS);
  double nPhi_nxTCS(nPhi * nxTCS);
  double nTheta_nyTCS(nTheta * nyTCS);
  double nPhi_nyTCS(nPhi * nyTCS);

  double diffPos_nTheta = diffPos * nTheta;
  double diffPos_nPhi   = diffPos * nPhi;

  HepVector residual(2);
  residual(1) = diffPos_nTheta * nTheta_nyTCS + diffPos_nPhi * nPhi_nyTCS;
  residual(2) = diffPos_nTheta * nTheta_nxTCS + diffPos_nPhi * nPhi_nxTCS;

// Calculate the measurement errors on the y-x plane of TCS.
// For endcap, undo the (theta,phi) -> (x,y) transformation done in
// common/com-klm/geom/KlmMOduleEndCap.cc for localErr() .

  HepSymMatrix hitCov(2, 0);
  if (p.address.inBarrel) {
    hitCov(1, 1) = localError[1] * nTheta_nyTCS * nTheta_nyTCS;
    hitCov(2, 2) = localError[0] * nPhi_nxTCS * nPhi_nxTCS;
  } else {
    double phi = hitPos.Phi();
    HepMatrix rotation(2, 2, 0);
    rotation(1, 1) =  cos(phi);
    rotation(1, 2) =  sin(phi);
    rotation(2, 1) = -rotation(1, 2);
    rotation(2, 2) =  rotation(1, 1);
    HepSymMatrix cov(2, 0);
    cov(1, 1) = localError[0];
    cov(1, 2) = 0.0;  // DIVOT off-diagonal term ???
    cov(2, 2) = localError[1];
    cov = cov.similarityT(rotation);
    hitCov(1, 1) = cov(1, 1) * nTheta_nyTCS * nTheta_nyTCS;
    hitCov(2, 2) = cov(2, 2) * nPhi_nxTCS * nPhi_nxTCS;
  }

// Now get the correction matrix: combined covariance of EXT and KLM hit.
// 1st dimension = nyTCS, 2nd dimension = nxTCS.

  HepSymMatrix correction = extCov.similarity(projectToTCS) + hitCov;

  int ierr;
  correction.invert(ierr);

  if (!ierr) {        // Matrix inversion succeeeded

// Track parameters (x,y,z,px,py,pz) before correction.

    HepVector extPar(6);
    extPar(1) = extPos.X();
    extPar(2) = extPos.Y();
    extPar(3) = extPos.Z();
    extPar(4) = extMom.X();
    extPar(5) = extMom.Y();
    extPar(6) = extMom.Z();

// Use the gain matrix formalism to get the corrected track parameters

    HepMatrix gain = (extCov * projectToTCS.T()) * correction;
    extPar += gain * residual;

    extPos.SetX(extPar(1));
    extPos.SetY(extPar(2));
    extPos.SetZ(extPar(3));
    extMom.SetX(extPar(4));
    extMom.SetY(extPar(5));
    extMom.SetZ(extPar(6));

// Project the corrected coordinate on the RPC plane along the track.
// Adjust the magnitude of the momentum with an ad hoc deceleration factor (??).

    // Project the corrected extrapolation to the plane that is tangent
    // to the hit-point's plane but at the original extrapolation's position, i.e.,
    // it leaves it in the same geometrical volume as it was originally.  Also,
    // the momentum magnitude is left unchanged.
    extDir = extMom.Unit();
    extPos += extDir * (((p.position - extPos) * nPerp) / (extDir * nPerp));
    extMom = p.momentum.Mag() * extDir;

// Calculate the new error matrix.

    HepSymMatrix HRH(correction.similarityT(projectToTCS));
    extCov -= HRH.similarity(extCov);

// Update the position, momentum and covariance of the point; calculate chi2

    p.position = extPos;
    p.momentum = extMom;
    p.covariance = extCov;

    correction = hitCov - extCov.similarity(projectToTCS);
    correction.invert(ierr);

    if (!ierr) {
      residual(1) = (hitPos - extPos) * nyTCS;
      residual(2) = (hitPos - extPos) * nxTCS;
      p.chi2 = correction.similarity(residual);
    }

  }     // if (inverted correction matrix)

  return;

}

double MuidModule::getPlaneError(HepSymMatrix& covariance, TVector3 nTrack, TVector3 nReadout)
{

  // DIVOT obsolete? if (!m_covValid) { return DEFAULT_ERROR; }

  double error = 1.0E12;

  // Construct 3 TCS axes.
  // z: nz( = nTrack )
  // x: nx --- unit vector perpendicular to nTrack and in the plane of nReadout and nTrack.
  // y: nz x nx

  double projection = nReadout * nTrack;
  double denom = 1.0 - projection * projection;

  if (denom > 1.0E-12) {    // Track is not parallel to the readout direction.

    double normalize = 1.0 / sqrt(denom);
    TVector3 nx = (nReadout - projection * nTrack) * normalize;
    TVector3 ny = nTrack.Cross(nx);

    HepMatrix rotation(3, 3, 0);
    rotation(1, 1) = nx.X();
    rotation(1, 2) = nx.Y();
    rotation(1, 3) = nx.Z();
    rotation(2, 1) = ny.X();
    rotation(2, 2) = ny.Y();
    rotation(2, 3) = ny.Z();
    rotation(3, 1) = nTrack.X();
    rotation(3, 2) = nTrack.Y();
    rotation(3, 3) = nTrack.Z();

    HepSymMatrix positionCovariance(covariance.sub(1, 3).similarity(rotation));
    error = sqrt(max(0.0, positionCovariance(1, 1)));    // variance along nx

  }

  return error;

}

//------------------------------------------------------------------------------
// finished with this track: write panther tables

void MuidModule::finishTrack(Muid* muid)
{

// Adjust if track went from barrel to endcap

  if (m_fromBarrelToEndcap) {
    if (m_lastEndcapLayerExt > m_lastEndcapLayerHit) {
      m_lastEndcapLayerExt--;
      if (m_numEndcapLayerExt > m_numEndcapLayerHit) {
        m_numEndcapLayerExt--;
      }
    }
  }

  muid->setChiSquared(m_chi2);
  muid->setDegreesOfFreedom(m_nPoint);
  muid->setExtLayerPattern(m_enteredPattern | m_matchedPattern);
  muid->setHitLayerPattern(m_matchedPattern);

  int outcome(0);
  int layerExt(m_lastBarrelLayerExt);
  if (m_lastBarrelLayerExt + m_lastEndcapLayerExt + 1 >= 0) {
    if (m_leftKLM) {
      outcome = 3;
      if (m_lastEndcapLayerExt >= 0) {
        outcome = 4;
        layerExt = m_lastEndcapLayerExt;
      }
    } else {
      outcome = 1;
      if (m_lastEndcapLayerExt >= 0) {
        outcome = 2;
        layerExt = m_lastEndcapLayerExt;
      }
    }
  }
  muid->setOutcome(outcome);
  muid->setBarrelExtLayer(m_lastBarrelLayerExt);
  muid->setEndcapExtLayer(m_lastEndcapLayerExt);
  muid->setBarrelHitLayer(m_lastBarrelLayerHit);
  muid->setEndcapHitLayer(m_lastEndcapLayerHit);
  muid->setExtLayer(m_lastBarrelLayerExt + m_lastEndcapLayerExt + 1);
  muid->setHitLayer((m_lastEndcapLayerHit == -1 ?
                     m_lastBarrelLayerHit :
                     m_lastBarrelLayerExt + m_lastEndcapLayerHit + 1));

  int layerDiff = muid->getExtLayer() - muid->getHitLayer();
  double chiSquaredReduced = Large;
  if (m_nPoint > 0) {
    chiSquaredReduced = m_chi2 / m_nPoint;
  }

// Do likelihood calculation

  double muon  = 0.0;
  double pion  = 0.0;
  double kaon  = 0.0;
  double miss  = 0.0;
  double junk  = 0.0;
  double denom = 0.0;
  if (outcome == 0) {
    miss = 1.0;
  } else {
    muon  = getPDF(MUON, outcome, layerExt, layerDiff, chiSquaredReduced);
    pion  = getPDF(PION, outcome, layerExt, layerDiff, chiSquaredReduced);
    kaon  = getPDF(KAON, outcome, layerExt, layerDiff, chiSquaredReduced);
    denom = muon + pion + kaon;
    if (denom < 1.0E-10) {
      denom = 1.0;
      muon  = pion = kaon = 0.0;
      junk  = 1.0;                    // anomaly: should never happen!!
    }
    muon /= denom;
    pion /= denom;
    kaon /= denom;
  }

  muid->setMuonPDFValue(muon);
  muid->setPionPDFValue(pion);
  muid->setKaonPDFValue(kaon);
  muid->setMissPDFValue(miss);
  muid->setJunkPDFValue(junk);
}

void MuidModule::getAddress(const G4String& topName, Address& address)
{

  // geant3 volume names were "klm_barrel_gas_#_#_##_#_phys"
  //                          "klm_endcap_gas_#_#_##_#_phys"
  //                          "klm_barrel_scint_#_#_##_#_phys"
  //                          "klm_endcap_scint_#_#_##_#_phys"
  // Need to use new volume names here
  address.inBarrel = topName.substr(4, 6) == "barrel";
  address.isRPC = (topName.substr(11, 3) == "gas");
  int i0 = (address.isRPC ? 0 : 2);
  address.isForward = topName.substr(i0 + 15, 1) == "F";
  address.sector = atoi(topName.substr(i0 + 17, 1).data());
  address.layer = atoi(topName.substr(i0 + 19, 2).data());
  address.isInnerPlane = true; // DIVOT

}

void MuidModule::fromG4eToPhasespace(G4ErrorFreeTrajState* state, CLHEP::HepSymMatrix& covariance)
{

  G4ErrorFreeTrajParam param = state->GetParameters();
  double p = 1.0 / (param.GetInvP() * GeV);     // in GeV/c
  double p2 = p * p;
  double lambda = param.GetLambda();            // in radians
  double phi = param.GetPhi();          // in radians
  double sinlambda = sin(lambda);
  double coslambda = cos(lambda);
  double sinphi = sin(phi);
  double cosphi = cos(phi);

  // Transformation Jacobian 6x5 from Geant4e 5x5 to Panther 6x6
  // Jacobian units are GeV/c, radians, cm
  // Geant4e covariance matrix units are GeV/c, radians, cm (!!!) - see PropagateError()
  // Panther covariance matrix units are GeV/c, cm

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
  G4ErrorSymMatrix temp = g4eCov.similarity(jacobian);

  // Ugh ...  Belle::HepSymMatrix is "same" type as G4ErrorSymMatrix
  for (int k = 1; k <= 6; k++) {
    for (int j = k; j <= 6; j++) {
      covariance(j, k) = temp.fast(j, k);
    }
  }

}

// Convert Panther covariance matrix (parameters x, y, z, px, py, pz)
// to Geant4e covariance matrix (parameters 1/p, lambda, phi, yt, zt)
// xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
// yT = -x * sin(phi) + y * cos(phi)
// zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)
// (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
// phi = atan( py / px )
// lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )

void MuidModule::fromPhasespaceToG4e(Point& p, G4ErrorTrajErr& g4eCov)
{

  G4ErrorSymMatrix temp(6, 0);
  for (int k = 1; k <= 6; k++) {
    for (int j = k; j <= 6; j++) {
      temp(j, k) = p.covariance.fast(j, k);
    }
  }

  double sinLambda = p.momentum.CosTheta();
  double cosLambda = sqrt(1.0 - sinLambda * sinLambda);
  double phi = p.momentum.Phi();
  double cosPhi = cos(phi);
  double sinPhi = sin(phi);
  double pInvSq = 1.0 / p.momentum.Mag2();
  double pInv   = sqrt(pInvSq);
  double pPerpInv = 1.0 / p.momentum.Perp();

  // Transformation Jacobian 5x6 from phase-space 6x6 to Geant4e 5x5
  // Jacobian units are GeV/c, radians, cm
  // Geant4e covariance matrix units are GeV/c, radians, cm (!!!) - see PropagateError()
  // Phase-space covariance matrix units are GeV/c, cm

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

  g4eCov = temp.similarity(jacobian);

}

void MuidModule::fillPDF(int hypothesis)
{

  // STUB
  double dx = 0.0;
  for (int l = MUON; l <= KAON; l++) {
    for (int k = 0; k < 4; k++) {
      for (int i = 0; i < kRchisq; i++) {
        // STUB
      }
      hypothesis++; // DIVOT to use this variable
      spline(kRchisq, dx, &fRchisq[l][k][0], &fRchisqD1[l][k][0],
             &fRchisqD2[l][k][0], &fRchisqD3[l][k][0]);
    }
  }
}

//____________________________________________________________________________

void MuidModule::spline(int n, double dx, double Y[], double B[],
                        double C[], double D[])
{

  // Generate the spline interpolation coefficients B, C, D to smooth out a
  // binned histogram. Restrictions:  equal-size bins. more than 3 bins.

  D[0] = dx;                                    // let's do it!
  C[1] = (Y[1] - Y[0]) / dx;
  for (int i = 1; i < n - 1; i++) {
    D[i]   = dx;
    B[i]   = dx * 4.0;
    C[i + 1] = (Y[i + 1] - Y[i]) / dx;
    C[i]   = C[i + 1] - C[i];
  }
  B[0]   = -dx;
  B[n - 1] = -dx;
  C[0]   = (C[2]   - C[1]) / 6.0;
  C[n - 1] = -(C[n - 2] - C[n - 3]) / 6.0;
  for (int i = 1; i < n; i++) {
    double temp = dx / B[i - 1];
    B[i] -= temp * dx;
    C[i] -= temp * C[i - 1];
  }
  C[n - 1] /= B[n - 1];
  for (int i = n - 2; i >= 0; i--) {
    C[i] = (C[i] - D[i] * C[i + 1]) / B[i];
  }
  B[n - 1] = (Y[n - 1] - Y[n - 2]) / dx + (C[n - 2] + C[n - 1] * 2.0) * dx;
  for (int i = 0; i < n - 1; i++) {
    B[i] = (Y[i + 1] - Y[i]) / dx - (C[i + 1] + C[i] * 2.0) * dx;
    D[i] = (C[i + 1] - C[i]) / dx;
    C[i] = C[i] * 3.0;
  }
  C[n - 1] = C[n - 1] * 3.0;
  D[n - 1] = D[n - 2];
}

//____________________________________________________________________________
double MuidModule::getPDF(int hypothesis, int outcome,
                          int lyr_ext, int lyr_dif, double chi2_red) const
{

  // hypothesis:  MUON or PION or KAON
  // outcome:  0=??, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
  // lyr_ext:  last layer that Ext track touched
  // lyr_dif:  difference between last Ext layer and last hit layer
  // chi2_red: reduced chi**2 of the transverse deviations of all associated
  //           hits from the corresponding Ext track crossings

  return getPDFRange(hypothesis, outcome, lyr_ext, lyr_dif) *
         getPDFRchisq(hypothesis, outcome, lyr_ext, chi2_red);

}

//____________________________________________________________________________

double MuidModule::getPDFRange(int hypothesis, int outcome, int lyr_ext,
                               int lyr_dif) const
{


  // hypothesis:  MUON or PION or KAON
  // outcome:  0=??, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
  // lyr_ext:  last layer that Ext track touched
  // lyr_dif:  difference between last Ext layer and last hit layer

  if (outcome <=  0) { return 0.0; }
  if (outcome >   4) { return 0.0; }
  if (lyr_ext <   1) { return 0.0; }
  if (lyr_ext >  15) { return 0.0; }
  if (lyr_dif <   0) { return 0.0; }
  if (lyr_dif >= kRange) { lyr_dif = kRange - 1; }

  return fRange[hypothesis][outcome - 1][lyr_ext - 1][lyr_dif];

}

//____________________________________________________________________________

double MuidModule::getPDFRchisq(int hypothesis, int outcome, int lyr_ext,
                                double chi2_red) const
{

  // hypothesis:  MUON or PION or KAON
  // outcome:  0=??, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
  // lyr_ext:  last layer that Ext track touched
  // chi2_red: reduced chi**2 of the transverse deviations of all associated
  //           hits from the corresponding Ext track crossings

  // Extract the probability density for a particular value of reduced
  // chi-squared by spline interpolation of the binned histogram values.
  // This avoids binning artifacts that were seen when the histogram
  // was sampled directly.

  if (outcome  <= 0) { return 0.0; }
  if (outcome  >  4) { return 0.0; }
  if (lyr_ext  <  1) { return 0.0; }
  if (lyr_ext  > 15) { return 0.0; }
  if (chi2_red <  0.0) { return 0.0; }

  double pdf;
  double area = fRchisqN[hypothesis][outcome - 1][lyr_ext - 1];
  if (chi2_red >= kRchisqMax) {
    pdf = area * fRchisq[hypothesis][outcome - 1][kRchisq - 1] + (1.0 - area);
  } else {
    int    i  = (int)(chi2_red / (kRchisqMax / kRchisq));
    double dx = chi2_red - i * (kRchisqMax / kRchisq);
    pdf = fRchisq[hypothesis][outcome - 1][i] +
          dx * (fRchisqD1[hypothesis][outcome - 1][i] +
                dx * (fRchisqD2[hypothesis][outcome - 1][i] +
                      dx * fRchisqD3[hypothesis][outcome - 1][i]));
    pdf = (pdf < 0.0) ? 0.0 : area * pdf;
  }
  return pdf;
}

