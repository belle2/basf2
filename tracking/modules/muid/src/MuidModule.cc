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
#include <tracking/modules/muid/MuidPar.h>
#include <tracking/dataobjects/Muid.h>
#include <tracking/dataobjects/MuidHit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <genfit/Exception.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/MagneticField.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtPhysicsList.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <bklm/geometry/GeometryPar.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <TMatrixDSym.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TVector3.h>
#include <TRandom.h> // DIVOT

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <genfit/Track.h>
#include <genfit/DetPlane.h>
#include <genfit/FieldManager.h>

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
#include <G4VPhysicalVolume.hh>
#include <G4FieldManager.hh>

using namespace std;
using namespace CLHEP;
using namespace Belle2;

#define TWOPI 6.283185482025146484375
#define M_PI_8 0.3926990926265716552734

#define Large 10.0E10

#define MUON 0
#define PION 1
#define KAON 2

REG_MODULE(Muid)

MuidModule::MuidModule() : Module(), m_extMgr(NULL)    // no ExtManager yet
{
  m_pdgCode.clear();
  setDescription("Identifies muons by extrapolating tracks from CDC to KLM using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("pdgCodes", m_pdgCode, "Positive-charge PDG codes for extrapolation hypotheses", m_pdgCode);
  addParam("BKLMHitsColName", m_bklmHitsColName, "Name of collection holding the reconstructed 2D hits in barrel KLM", string(""));
  addParam("EKLMHitsColName", m_eklmHitsColName, "Name of collection holding the reconstructed 2D hits in endcap KLM", string(""));
  addParam("TracksColName", m_tracksColName, "Name of collection holding the reconstructed tracks", string(""));
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

  // Convert from GeV to GEANT4 energy units (MeV); avoid negative values
  m_minPt = max(0.0, m_minPt) * GeV;
  m_minKE = max(0.0, m_minKE) * GeV;

  // Define the list of BKLM/EKLM sensitive volumes in the geant4 geometry
  registerVolumes();

  // Define the geant4e extrapolation Manager.
  m_extMgr = Simulation::ExtManager::GetManager();

  // See if muid will coexist with geant4 simulation and/or ext extrapolation
  if (m_extMgr->PrintG4State() == G4String("G4State_PreInit")) {
    B2INFO("muid::initialize:  I will run without simulation")
    m_runMgr = NULL;
    m_trk    = NULL;
    m_stp    = NULL;
    if (m_extMgr->PrintExtState() == G4String("G4ErrorState_PreInit")) {
      B2INFO("muid::initialize:  I will call InitGeant4e")
      // Create the magnetic field for the geant4e extrapolation
      Simulation::MagneticField* magneticField = new Simulation::MagneticField();
      G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
      fieldManager->SetDetectorField(magneticField);
      fieldManager->CreateChordFinder(magneticField);
      // Tell geant4e about the detector and bare-bones physics list
      m_extMgr->SetUserInitialization(new DetectorConstruction());
      G4Region* region = (*(G4RegionStore::GetInstance()))[0];
      region->SetProductionCuts(G4ProductionCutsTable::GetProductionCutsTable()->GetDefaultProductionCuts());
      m_extMgr->SetUserInitialization(new Simulation::ExtPhysicsList);
      m_extMgr->InitGeant4e();
    } else {
      B2INFO("muid::initialize:  I will not call InitGeant4e since it has already been initialized")
    }
  } else {
    B2INFO("muid::initialize:  I will coexist with simulation")
    m_runMgr = G4RunManager::GetRunManager();
    m_trk    = const_cast<G4UserTrackingAction*>(m_runMgr->GetUserTrackingAction());
    m_stp    = const_cast<G4UserSteppingAction*>(m_runMgr->GetUserSteppingAction());
    if (m_extMgr->PrintExtState() == G4String("G4ErrorState_PreInit")) {
      B2INFO("muid::initialize:  I will call InitGeant4e")
      m_extMgr->InitGeant4e();
      G4StateManager::GetStateManager()->SetNewState(G4State_Idle);
    } else {
      B2INFO("muid::initialize:  I will not call InitGeant4e since it has already been initialized")
    }
  }

  // Redefine muid's step length, magnetic field step limitation (fraction of local curvature radius),
  // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  G4double maxStep = ((m_maxStep == 0.0) ? 10.0 : std::min(10.0, m_maxStep)) * cm;
  char line[80];
  std::sprintf(line, "/geant4e/limits/stepLength %8.2f mm", maxStep);
  G4UImanager::GetUIpointer()->ApplyCommand(line);
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  GearDir bklmContent = GearDir("/Detector/DetectorComponent[@name=\"BKLM\"]/Content/");
  GearDir eklmContent = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/");
  m_BarrelHalfLength = bklmContent.getLength("HalfLength") * cm; // in G4 units (mm)
  m_EndcapHalfLength = 0.5 * eklmContent.getLength("Endcap/Length") * cm; // in G4 units (mm)
  m_OffsetZ = bklmContent.getLength("OffsetZ") * cm; // in G4 units (mm)
  double minZ = m_OffsetZ - (m_BarrelHalfLength + 2.0 * m_EndcapHalfLength);
  double maxZ = m_OffsetZ + (m_BarrelHalfLength + 2.0 * m_EndcapHalfLength);
  double rMax = bklmContent.getLength("OuterRadius") * cm / cos(M_PI / bklmContent.getNumberNodes("Sectors/Forward/Sector"));
  m_target = new Simulation::ExtCylSurfaceTarget(rMax, minZ, maxZ);
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_target);

  m_BarrelHalfLength /= cm;  // in G4e units (cm)
  m_EndcapHalfLength /= cm;  // in G4e units (cm)
  m_OffsetZ /= cm;           // in G4e units (cm)
  m_BarrelMinR = bklmContent.getLength("Layers/InnerRadius");  // in G4e units (cm)
  m_EndcapMinR = eklmContent.getLength("Endcap/InnerR");       // in G4e units (cm) 125.0 cm --> 130.5 cm
  m_EndcapMaxR = eklmContent.getLength("Endcap/OuterR");       // in G4e units (cm) 290.0 cm --> 332.0 cm
  m_EndcapMiddleZ = m_BarrelHalfLength + m_EndcapHalfLength;   // in G4e units (cm)

  // Measurement uncertainties and acceptance windows
  m_maxDistCM = 25.0;         // in G4e units (cm) max distance between KLM hit and extrapolation crossing
  m_maxDistSIGMA = 5.0;       // ditto (in sigmas)
  double width = eklmContent.getLength("Endcap/Layer/Sector/Plane/Strips/Width");
  m_EndcapScintVariance = width * width / 12.0;
  width = bklmContent.getLength("Module/Scintillator/Width"); // in G4e units (cm)
  m_BarrelScintVariance = width * width / 12.0;
  int nBarrelLayers = bklmContent.getNumberNodes("Layers/Layer");
  for (int layer = 1; layer <= nBarrelLayers; ++layer) {
    std::sprintf(line, "Layers/Layer[@layer=\"%d\"]/PhiStrips/Width", layer);
    width = bklmContent.getLength(line); // in G4e units (cm)
    m_BarrelPhiStripVariance[layer - 1] = width * width / 12.0;
    std::sprintf(line, "Layers/Layer[@layer=\"%d\"]/ZStrips/Width", layer);
    width = bklmContent.getLength(line); // in G4e units (cm)
    m_BarrelZStripVariance[layer - 1] = width * width / 12.0;
  }

  // KLM geometry (for associating KLM hit with extrapolated crossing point)

  m_LastActiveBarrelLayer = nBarrelLayers - 1; // zero-based counting
  for (int layer = 1; layer <= nBarrelLayers; ++layer) {
    m_BarrelModuleMiddleRadius[layer - 1] = bklm::GeometryPar::instance()->getModuleMiddleRadius(layer); // in G4e units (cm)
  }
  double dz(eklmContent.getLength("Endcap/Layer/ShiftZ")); // in G4e units (cm)
  double z0(eklmContent.getLength("Endcap/PositionZ") - m_OffsetZ + dz - 0.5 * eklmContent.getLength("Endcap/Layer/Length")); // in G4e units (cm)
  int nEndcapLayers = eklmContent.getInt("Endcap/nLayer");
  m_LastActiveEndcapLayer = nEndcapLayers - 1; // zero-based counting
  for (int layer = 1; layer <= nEndcapLayers; ++layer) {
    m_EndcapModuleMiddleZ[layer - 1] = z0 + dz * (layer - 1); // in G4e units (cm)
  }
  for (int sector = 1; sector <= 8; ++sector) {
    double phi = M_PI_4 * (sector - 1);
    m_BarrelSectorPerp[sector - 1].SetX(cos(phi));
    m_BarrelSectorPerp[sector - 1].SetY(sin(phi));
    m_BarrelSectorPerp[sector - 1].SetZ(0.0);
    m_BarrelSectorPhi[sector - 1].SetX(-sin(phi));
    m_BarrelSectorPhi[sector - 1].SetY(cos(phi));
    m_BarrelSectorPhi[sector - 1].SetZ(0.0);
  }

  // Hypotheses for extrapolation (default is muon only)
  if (m_pdgCode.empty()) {
    m_chargedStable.push_back(Const::muon);
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
    if (m_chargedStable.empty()) B2ERROR("Module muid initialize(): no valid PDG codes for extrapolation")
    }

  for (unsigned i = 0; i < m_chargedStable.size(); ++i) {
    B2INFO("Module muid initialize(): hypothesis for PDG code "
           << m_chargedStable[i].getPDGCode() << " and its antiparticle will be extrapolated")
  }

  int expNo = 0;  // DIVOT
  m_muonPar = new MuidPar(expNo, "Muon");
  m_pionPar = new MuidPar(expNo, "Pion");
  m_kaonPar = new MuidPar(expNo, "Kaon");
  m_protonPar = new MuidPar(expNo, "Proton");
  m_electronPar = new MuidPar(expNo, "Electron");

  // Register output and relation arrays' persistence
  StoreArray<Muid>::registerPersistent();
  StoreArray<MuidHit>::registerPersistent();
  RelationArray::registerPersistent<Track, Muid>();
  RelationArray::registerPersistent<Track, MuidHit>();

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
  // Do extrapolation for each hypotheses of each reconstructed track.

  StoreArray<BKLMHit2d> bklmHits(m_bklmHitsColName);
  StoreArray<EKLMHit2d> eklmHits(m_eklmHitsColName);
  StoreArray<Track> tracks(m_tracksColName);
  StoreArray<Muid> muids(m_muidsColName);
  StoreArray<MuidHit> muidHits(m_muidHitsColName);
  RelationArray trackToMuid(tracks, muids);
  RelationArray trackToMuidHits(tracks, muidHits);

  G4Point3D position;
  G4Vector3D momentum;
  G4ErrorTrajErr covG4e(5, 0);


  for (int t = 0; t < tracks.getEntries(); ++t) {

    for (unsigned int hypothesis = 0; hypothesis < m_chargedStable.size(); ++hypothesis) {

      Const::ChargedStable chargedStable = m_chargedStable[hypothesis];
      int pdgCode = chargedStable.getPDGCode();
      if (chargedStable == Const::electron || chargedStable == Const::muon) pdgCode = -pdgCode;

      Muid* muid = new(muids.nextFreeAddress()) Muid(pdgCode); // pdgCode doesn't know charge yet
      trackToMuid.add(t, muids.getEntries() - 1);

      const TrackFitResult* trackFit = tracks[t]->getTrackFitResult(chargedStable);
      if (!trackFit) {
        B2ERROR("Muid::event(): no valid TrackFitResult for PDGcode " <<
                chargedStable.getPDGCode() << ": extrapolation not possible")
        continue;
      }

      const genfit::Track* gfTrack = DataStore::getRelated<genfit::Track>(trackFit);
      if (!gfTrack) {
        B2ERROR("Muid::event(): no relation of TrackFitResult with genfit::Track for PDGcode " <<
                chargedStable.getPDGCode() << ": extrapolation not possible")
        continue;
      }

      pdgCode *= int(gfTrack->getFittedState().getCharge());
      muid->setPDGCode(pdgCode);

      getStartPoint(gfTrack, pdgCode, position, momentum, covG4e);
      if (momentum.perp() <= m_minPt) continue;
      if (m_target->GetDistanceFromPoint(position) < 0.0) continue;
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
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (length > 0.0) {
          if (createHit(state, t, pdgCode, muidHits, trackToMuidHits, bklmHits, eklmHits)) {
            // Force geant4e to update its G4Track from the Kalman-updated state
            m_extMgr->GetPropagator()->SetStepN(0);
          }
          m_tof += step->GetDeltaTime();
        }
        // Post-step momentum too low?
        if (errCode || (track->GetMomentum().mag() < minP)) {
          break;
        }
        // Reached the target boundary?
        if (m_target->GetDistanceFromPoint(track->GetPosition()) < 0.0) {
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
  delete m_target;
  delete m_bklm_enter;
  delete m_eklm_enter;

  delete m_muonPar;
  delete m_pionPar;
  delete m_kaonPar;
  delete m_protonPar;
  delete m_electronPar;

}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void MuidModule::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("Module muid registerVolumes(): No geometry defined. Please create the geometry first.")
  }

  m_bklm_enter = new vector<G4VPhysicalVolume*>;
  m_eklm_enter = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // see belle2/run/volname3.txt:
    // Barrel KLM: BKLM.Layer**GasPhysical for RPCs or BKLM.Layer**ChimneyGasPhysical for RPCs
    //             BKLM.ScintType*Physical for scintillator strips
    if (name.substr(0, 5) == "BKLM.") {
      if ((name.find("ScintType") != string::npos) ||
          (name.find("GasPhysical") != string::npos)) {
        m_bklm_enter->push_back(*iVol);
      }
    }
    // Endcap KLM: Endcap_{1,2} and
    // Sensitive_Strip_StripVolume_{1..75}_Plane_{1,2}_Sector_{1..4}_Layer_{1..14}_Endcap_{1,2}
    if ((name.substr(0, 7) == "Endcap_") || (name.substr(0, 27) == "Sensitive_Strip_StripVolume")) {
      m_eklm_enter->push_back(*iVol);
    }
  }

}

void MuidModule::getStartPoint(const genfit::Track* gfTrack, int pdgCode,
                               G4Point3D& position, G4Vector3D& momentum, G4ErrorTrajErr& covG4e)
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
    m_tof = pathLength * (sqrt(lastMomMag * lastMomMag + mass * mass) / (lastMomMag * c_light / (cm / ns)));

    covG4e = fromPhasespaceToG4e(lastMomentum, lastCov); // in Geant4e units (GeV/c, cm)
    position.setX(lastPosition.X()*cm); // in Geant4 units (mm)
    position.setY(lastPosition.Y()*cm);
    position.setZ(lastPosition.Z()*cm);
    momentum.setX(lastMomentum.X()*GeV);  // in Geant4 units (MeV/c)
    momentum.setY(lastMomentum.Y()*GeV);
    momentum.setZ(lastMomentum.Z()*GeV);
  }

  catch (genfit::Exception& e) {
    B2WARNING("Ext::getStartPoint() caught genfit exception for " << (firstLast ? "first" : "last") << " point on track; will not extrapolate. " << e.what())
    // Do not extrapolate this track by forcing minPt cut to fail
    momentum.setX(0.0);
    momentum.setY(0.0);
    momentum.setZ(0.0);
  }

  // Keep track of geometrical state during one track's extrapolation
  m_fromBarrelToEndcap = false;
  m_wasInBarrel = false;
  m_wasInEndcap = false;
  m_firstBarrelLayer = 0;            // ratchet outward when looking for matching barrel hits
  m_firstEndcapLayer = 0;            // ratchet outward when looking for matching endcap hits
  m_inBarrel = false;                // invalidate prior information about previous track-layer intersection
  m_isForward = false;               // ditto
  m_sector = -1;                     // ditto
  m_layer = -1;                      // ditto
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
                           StoreArray<MuidHit>& muidHits, RelationArray& trackToMuidHits,
                           const StoreArray<BKLMHit2d>& bklmHits, const StoreArray<EKLMHit2d>& eklmHits)
{

  G4StepPoint* stepPoint = state->GetG4Track()->GetStep()->GetPreStepPoint();
  TVector3 position(stepPoint->GetPosition().x() / cm, stepPoint->GetPosition().y() / cm, stepPoint->GetPosition().z() / cm);
  double r = position.Perp();
  double z = fabs(position.z() - m_OffsetZ);

  bool isInEndcap((r > m_EndcapMinR) && (fabs(z - m_EndcapMiddleZ) < m_EndcapHalfLength));
  bool isInBarrel((r > m_BarrelMinR) && (z < m_BarrelHalfLength));

  if (m_wasInBarrel && isInEndcap) {
    m_fromBarrelToEndcap = true;
  }

  if (!(m_wasInEndcap || m_wasInBarrel || isInEndcap || isInBarrel)) {
    m_position = position;
    return false;
  }

  Point p;
  p.chi2 = -1.0;

  // Did the track cross a KLM detector layer?

  if (m_wasInBarrel || isInBarrel) {
    if (findBarrelIntersection(p, m_position, position)) {
      p.momentum.SetX(stepPoint->GetMomentum().x() / GeV);
      p.momentum.SetY(stepPoint->GetMomentum().y() / GeV);
      p.momentum.SetZ(stepPoint->GetMomentum().z() / GeV);
      p.covariance.ResizeTo(6, 6);
      p.covariance = fromG4eToPhasespace(state);
      if (findMatchingBarrelHit(p, bklmHits)) {
        m_enteredPattern |= (0x00000001 << p.layer);
        if (m_lastBarrelLayerExt < p.layer) {
          m_lastBarrelLayerExt = p.layer;
          m_numBarrelLayerExt++;
        }
        m_matchedPattern |= (0x00000001 << p.layer);
        if (m_lastBarrelLayerHit < p.layer) {
          m_lastBarrelLayerHit = p.layer;
          m_numBarrelLayerHit++;
        }
        position = p.position;
        m_inBarrel = p.inBarrel;
        m_isForward = p.isForward;
        m_sector = p.sector;
        m_layer = p.layer;
        // adjust geant4e's position, momentum and covariance based on matching hit
        if (p.chi2 >= 0.0) {
          G4Point3D newPos(p.position.X()*cm, p.position.Y()*cm, p.position.Z()*cm);
          state->SetPosition(newPos);
          G4Vector3D newMom(p.momentum.X()*GeV, p.momentum.Y()*GeV, p.momentum.Z()*GeV);
          state->SetMomentum(newMom);
          state->SetError(fromPhasespaceToG4e(p.momentum, p.covariance));
          double extTime = 0.0;  // DIVOT should be extrapolated-track time from IP to BKLM
          double hitTime = 0.0;  // DIVOT should be measured hit time
          new(muidHits.nextFreeAddress())
          MuidHit(pdgCode, p.inBarrel, p.isForward, p.sector, p.layer, p.position, p.positionAtHitPlane, extTime, hitTime, p.chi2);
          trackToMuidHits.add(trackID, muidHits.getEntries() - 1);
        }
      } else {
        // record the no-hit intersection if it was inside the sensitive volume
        G4VPhysicalVolume* preVol = stepPoint->GetTouchableHandle()->GetVolume();
        vector<G4VPhysicalVolume*>::iterator j;
        for (j = m_bklm_enter->begin(); (j != m_bklm_enter->end()) && (*j != preVol); ++j) {}
        if (j != m_bklm_enter->end()) {        // entered a BKLM sensitive volume?
          m_enteredPattern |= (0x00000001 << p.layer);
          if (m_lastBarrelLayerExt < p.layer) {
            m_lastBarrelLayerExt = p.layer;
            m_numBarrelLayerExt++;
          }
        }
      }
    }
  }

  if (m_wasInEndcap || isInEndcap) {
    if (findEndcapIntersection(p, m_position, position)) {
      p.momentum.SetX(stepPoint->GetMomentum().x() / GeV);
      p.momentum.SetY(stepPoint->GetMomentum().y() / GeV);
      p.momentum.SetZ(stepPoint->GetMomentum().z() / GeV);
      p.covariance.ResizeTo(6, 6);
      p.covariance = fromG4eToPhasespace(state);
      if (findMatchingEndcapHit(p, eklmHits)) {
        m_enteredPattern |= (0x00008000 << p.layer);
        if (m_lastEndcapLayerExt < p.layer) {
          m_lastEndcapLayerExt = p.layer;
          m_numEndcapLayerExt++;
        }
        m_matchedPattern |= (0x00008000 << p.layer);
        if (m_lastEndcapLayerHit < p.layer) {
          m_lastEndcapLayerHit = p.layer;
          m_numEndcapLayerHit++;
        }
        position = p.position;
        m_inBarrel = p.inBarrel;
        m_isForward = p.isForward;
        m_sector = p.sector;
        m_layer = p.layer;
        // adjust geant4e's position, momentum and covariance based on matching hit
        if (p.chi2 >= 0.0) {
          G4Point3D newPos(p.position.X()*cm, p.position.Y()*cm, p.position.Z()*cm);
          state->SetPosition(newPos);
          G4Vector3D newMom(p.momentum.X()*GeV, p.momentum.Y()*GeV, p.momentum.Z()*GeV);
          state->SetMomentum(newMom);
          state->SetError(fromPhasespaceToG4e(p.momentum, p.covariance));
          double extTime = 0.0;  // DIVOT should be extrapolated-track time from IP to BKLM
          double hitTime = 0.0;  // DIVOT should be measured hit time
          new(muidHits.nextFreeAddress())
          MuidHit(pdgCode, p.inBarrel, p.isForward, p.sector, p.layer, p.position, p.positionAtHitPlane, extTime, hitTime, p.chi2);
          trackToMuidHits.add(trackID, muidHits.getEntries() - 1);
        }
      } else {
        // record the no-hit intersection if it was inside the sensitive volume
        G4VPhysicalVolume* preVol = stepPoint->GetTouchableHandle()->GetVolume();
        vector<G4VPhysicalVolume*>::iterator j;
        for (j = m_eklm_enter->begin(); (j != m_eklm_enter->end()) && (*j != preVol); ++j) {}
        if (j != m_eklm_enter->end()) {        // entered an EKLM sensitive volume?
          m_enteredPattern |= (0x00008000 << p.layer);
          if (m_lastEndcapLayerExt < p.layer) {
            m_lastEndcapLayerExt = p.layer;
            m_numEndcapLayerExt++;
          }
        }
      }
    }
  }

  m_position = position;
  m_wasInBarrel = isInBarrel;
  m_wasInEndcap = isInEndcap;
  return (p.chi2 >= 0.0); // true if the track's state has been updated by the Kalman filter

}

bool MuidModule::findBarrelIntersection(Point& p, const TVector3& oldPos, const TVector3& newPos)
{

  // Be generous: allow outward-moving intersection to be in the dead space between
  // largest sensitive-volume Z and m_BarrelHalfLength, not necessarily in a geant4 sensitive volume

  if (fabs(newPos.z() - m_OffsetZ) > m_BarrelHalfLength) { return false; }

  int sector[2];
  double phi = oldPos.Phi();
  if (phi < 0.0) { phi += TWOPI; }
  if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
  sector[0] = (int)((phi + M_PI_8) / M_PI_4);

  phi = newPos.Phi();
  if (phi < 0.0) { phi += TWOPI; }
  if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
  sector[1] = (int)((phi + M_PI_8) / M_PI_4);

  for (int j = 0; j < 2; j++) {
    if ((j == 0) && (sector[0] == sector[1])) { continue; }
    double oldR = oldPos * m_BarrelSectorPerp[sector[j]];
    double newR = newPos * m_BarrelSectorPerp[sector[j]];
    double diffR = newR - oldR;
    for (int layer = m_firstBarrelLayer; layer <= m_LastActiveBarrelLayer; layer++) {
      if (newR <  m_BarrelModuleMiddleRadius[layer]) { break; }
      if (oldR <= m_BarrelModuleMiddleRadius[layer]) {
        if (fabs(diffR) > 1.0E-12) {
          p.position = oldPos + (newPos - oldPos) * ((m_BarrelModuleMiddleRadius[layer] - oldR) / diffR);
        } else {
          p.position = 0.5 * (oldPos + newPos);
        }
        phi = p.position.Phi();
        if (phi < 0.0) { phi += TWOPI; }
        if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
        if (sector[j] == (int)((phi + M_PI_8) / M_PI_4)) {
          m_firstBarrelLayer = layer + 1; // ratchet outward for next call's loop starting value
          p.layer = layer;
          p.sector = sector[j];
          p.inBarrel = true;
          p.isForward = p.position.Z() > m_OffsetZ;
          return true;
        }
        break;
      }
    }
  }

  return false;

}

bool MuidModule::findEndcapIntersection(Point& p, const TVector3& oldPos, const TVector3& newPos)
{

  // Be generous: allow intersection to be in the dead space between m_EndcapMinR and innermost
  // sensitive-volume radius or between outermost sensitive-volume radius and m_EndcapMaxR,
  // not necessarily in a geant4 sensitive volume

  if (oldPos.Perp() > m_EndcapMaxR) { return false; }
  if (newPos.Perp() < m_EndcapMinR) { return false; }

  double oldZ = fabs(oldPos.Z() - m_OffsetZ);
  double newZ = fabs(newPos.Z() - m_OffsetZ);

  for (int layer = m_firstEndcapLayer; layer <= m_LastActiveEndcapLayer; layer++) {
    if (newZ <  m_EndcapModuleMiddleZ[layer]) { break; }
    if (oldZ <= m_EndcapModuleMiddleZ[layer]) {
      double diffZ = newZ - oldZ;
      if (fabs(diffZ) > 1.0E-12) {
        p.position = oldPos + (newPos - oldPos) * ((m_EndcapModuleMiddleZ[layer] - oldZ) / diffZ);
      } else {
        p.position = 0.5 * (oldPos + newPos);
      }
      m_firstEndcapLayer = layer + 1; // ratchet outward for next call's loop starting value
      p.inBarrel = false;
      p.isForward = p.position.Z() > m_OffsetZ;
      p.layer = layer;
      double phi = p.position.Phi();
      if (phi < 0.0) { phi += TWOPI; }
      if (p.isForward) {
        phi = M_PI - phi;
        if (phi < 0.0) { phi += TWOPI; }
      }
      p.sector = (int)(phi / M_PI_2); // my calculation; matches EKLM-geometry calculation
      p.inBarrel = false;
      p.isForward = p.position.Z() > m_OffsetZ;
      return true;
    }
  }

  return false;

}

bool MuidModule::findMatchingBarrelHit(Point& p, const StoreArray<BKLMHit2d>& bklmHits)
{

  double diffBest = 1.0E12;
  double localVariance[2] = {m_BarrelScintVariance, m_BarrelScintVariance};

  BKLMHit2d* hitBest = NULL;
  for (int h = 0; h < bklmHits.getEntries(); ++h) {
    BKLMHit2d* hit = bklmHits[h];
    if (hit->getLayer() - 1 != p.layer) { continue; }
    if (hit->getStatus() & STATUS_OUTOFTIME) { continue; }
    TVector3 diff = hit->getGlobalPosition() - p.position;
    // allow hit in adjacent sector but only if crossing is near this sector's edge
    if (fabs(diff * m_BarrelSectorPerp[hit->getSector() - 1]) > 20.0) { continue; }
    double diffMag = diff.Mag();
    if (diffMag < diffBest) {
      diffBest = diffMag;
      hitBest = hit;
    }
  }

  if (hitBest == NULL) { return false; }

  p.isForward = hitBest->isForward();
  p.sector = hitBest->getSector() - 1;
  double varTrack = getPlaneVariance(p);
  double varHit = localVariance[0] + localVariance[1];
  if (hitBest->isInRPC()) {
    localVariance[0] = m_BarrelPhiStripVariance[p.layer];
    localVariance[1] = m_BarrelZStripVariance[p.layer];
  }
  if (diffBest <= max(sqrt(varTrack + varHit) * m_maxDistSIGMA, m_maxDistCM)) {
    adjustIntersection(p, localVariance, hitBest->getGlobalPosition());
    if (p.chi2 >= 0.0) {
      m_chi2 += p.chi2;
      m_nPoint++;
    }
    hitBest->setStatus(STATUS_ONTRACK);
    return true;
  }

  return false;

}

bool MuidModule::findMatchingEndcapHit(Point& p, const StoreArray<EKLMHit2d>& eklmHits)
{

  double diffBest = 1.0E12;
  double localVariance[2] = {m_EndcapScintVariance, m_EndcapScintVariance};

  EKLMHit2d* hitBest = NULL;
  for (int h = 0; h < eklmHits.getEntries(); ++h) {
    EKLMHit2d* hit = eklmHits[h];
    if (hit->getLayer() - 1 != p.layer) { continue; }
    // DIVOT no getStatus() if (hit->getStatus() & STATUS_OUTOFTIME) { continue; }
    TVector3 diff = hit->getPosition() - p.position;
    double diffMag = diff.Mag();
    if (diffMag < diffBest) {
      diffBest = diffMag;
      hitBest = hit;
    }
  }

  if (hitBest == NULL) { return false; }

  p.isForward = (hitBest->getEndcap() == 1);
  p.sector = hitBest->getSector() - 1;
  double varTrack = getPlaneVariance(p);
  double varHit = localVariance[0] + localVariance[1];
  if (diffBest <= max(sqrt(varTrack + varHit) * m_maxDistSIGMA, m_maxDistCM)) {
    adjustIntersection(p, localVariance, hitBest->getPosition());
    if (p.chi2 >= 0.0) {
      m_chi2 += p.chi2;
      m_nPoint++;
    }
    // DIVOT no setStatus() hitBest->setStatus(STATUS_ONTRACK);
    return true;
  }

  return false;

}

void MuidModule::adjustIntersection(Point& p, const double localVariance[2], const TVector3& hitPos)
{

// Use the gain matrix formalism to get the corrected track parameters.
// R. Fruhwirth, Application of Kalman Filtering, NIM A262 (1987) 444-450
// Equations (7)
// x_k^{k-1} = extPar[] 6 elements before filtering
// C_k^{k-1} = extCov[] 6x6 symmetric before filtering
// r_k^{k-1} = residual[] 2 elements before filtering
// h_k = 2x6 projects cartesian coordinates to measurement-plane coordinates
// H_k = @h_k/@x = jacobian[] 2x6 Jacobian of projection to measurement plane
// R_k^{k-1} = correction[] 2x2 before Invert()
// G_k = R^(-1) = correction[] 2x2 after Invert()
// K_k = gain[] 6x2 Kalman gain matrix
// x_k = extPar[] 6 elements after filtering
// C_k = extCov[] 6x6 symmetric after filtering
// r_k = residual[] 2 elements after filtering
// Use the relation K*H*C = (C*H^T*R^-1)*H*C = C*(H^T*R^-1*H)*C^T

  TVector3 extPos(p.position);
  TVector3 extMom(p.momentum);
  TVector3 extDir(extMom.Unit());
  TVector3 diffPos(hitPos - extPos);
  TMatrixDSym extCov(p.covariance);

// Track parameters (x,y,z,px,py,pz) before correction

  TVectorD extPar(6);
  extPar[0] = extPos.X();
  extPar[1] = extPos.Y();
  extPar[2] = extPos.Z();
  extPar[3] = extMom.X();
  extPar[4] = extMom.Y();
  extPar[5] = extMom.Z();

  TVector3 nA;  // unit vector normal to the readout plane
  TVector3 nB;  // unit vector along phi- or x-readout direction (for barrel or endcap)
  TVector3 nC;  // unit vector along z- or y-readout direction (for barrel or endcap)
  if (p.inBarrel) {
    nA = m_BarrelSectorPerp[p.sector];
    nB = m_BarrelSectorPhi[p.sector];
    nC = TVector3(0.0, 0.0, 1.0);
  } else {
    double out = (p.isForward ? 1.0 : -1.0);
    nA = TVector3(0.0, 0.0, out);
    nB = TVector3(out, 0.0, 0.0);
    nC = TVector3(0.0, out, 0.0);
  }

// Don't adjust the extrapolation if the track is nearly tangent to the readout plane.

  double extDirA = extDir * nA;
  if (fabs(extDirA) < 1.0E-6) { return; }
  double extDirBA = extDir * nB / extDirA;
  double extDirCA = extDir * nC / extDirA;

// Move the extrapolated coordinate (at most a tiny amount!) to the plane of the hit.

//*  TVector3 move = extDir * ((diffPos * nA) / (extDir * nA));
//*  extPos += move;
//*  diffPos -= move;
//*  p.positionAtHitPlane.SetX(extPos.X());
//*  p.positionAtHitPlane.SetY(extPos.Y());
//*  p.positionAtHitPlane.SetZ(extPos.Z());
  TVector3 move = extPos + extDir * ((diffPos * nA) / extDirA);
  p.positionAtHitPlane.SetX(move.X());
  p.positionAtHitPlane.SetY(move.Y());
  p.positionAtHitPlane.SetZ(move.Z());

// Projection jacobian onto the nB-nC measurement plane

  TMatrixD jacobian(2, 6);
  jacobian[0][0] = nB.X()  - nA.X() * extDirBA;
  jacobian[0][1] = nB.Y()  - nA.Y() * extDirBA;
  jacobian[0][2] = nB.Z()  - nA.Z() * extDirBA;
  jacobian[1][0] = nC.X()  - nA.X() * extDirCA;
  jacobian[1][1] = nC.Y()  - nA.Y() * extDirCA;
  jacobian[1][2] = nC.Z()  - nA.Z() * extDirCA;

// Residuals of EXT track and KLM hit on the nB-nC measurement plane

  TVectorD residual(2);
  residual[0] = diffPos.X() * jacobian[0][0] + diffPos.Y() * jacobian[0][1] + diffPos.Z() * jacobian[0][2];
  residual[1] = diffPos.X() * jacobian[1][0] + diffPos.Y() * jacobian[1][1] + diffPos.Z() * jacobian[1][2];

// Measurement errors in the detector plane

  TMatrixDSym hitCov(2);
  hitCov[0][0] = localVariance[0];
  hitCov[1][1] = localVariance[1];

// Now get the correction matrix: combined covariance of EXT and KLM hit.
// 1st dimension = nB, 2nd dimension = nC.

  TMatrixDSym correction(extCov);  // 6x6
  correction = correction.Similarity(jacobian) + hitCov; // Similarity() changes correction to 2x2!

  double determinant = 0.0;
  correction.Invert(&determinant);

  if (determinant != 0.0) { // Matrix inversion succeeeded

    TMatrixD gain(6, 2);
    gain.MultT(extCov, jacobian);
    gain *= correction;
    TMatrixDSym HRH(correction.SimilarityT(jacobian));

// Do the Kalman filtering

    extCov -= HRH.Similarity(extCov);
    extPar += gain * residual;
    extPos.SetX(extPar[0]);
    extPos.SetY(extPar[1]);
    extPos.SetZ(extPar[2]);
    extMom.SetX(extPar[3]);
    extMom.SetY(extPar[4]);
    extMom.SetZ(extPar[5]);

// Project the corrected extrapolation to the plane that is tangent
// to the hit-point's plane but at the original extrapolation's position, i.e.,
// it leaves it in the same geometrical volume as it was originally.  Also,
// the momentum magnitude is left unchanged (otherwise, it would vary by ~1%).

    extDir = extMom.Unit();
//*    extPos += extDir * (((p.position - extPos) * nA) / (extDir * nA));
    extMom = p.momentum.Mag() * extDir;

// Update the position, momentum and covariance of the point; calculate chi2

    p.position = extPos;
    p.momentum = extMom;
    p.covariance = extCov;

// Calculate chi-squared increment

    correction = extCov;
    correction = hitCov - correction.Similarity(jacobian);
    correction.Invert(&determinant);

    if (determinant != 0.0) {
      diffPos = hitPos - extPos;
      residual[0] = diffPos.X() * jacobian[0][0] + diffPos.Y() * jacobian[0][1] + diffPos.Z() * jacobian[0][2];
      residual[1] = diffPos.X() * jacobian[1][0] + diffPos.Y() * jacobian[1][1] + diffPos.Z() * jacobian[1][2];
      p.chi2 = correction.Similarity(residual);
    }

  }     // if (inverted correction matrix)

  return;

}

double MuidModule::getPlaneVariance(const Point& p)
{

  TVector3 nA;  // unit vector normal to the readout plane
  TVector3 nB;  // unit vector along phi- or x-readout direction (for barrel or endcap)
  TVector3 nC;  // unit vector along z- or y-readout direction (for barrel or endcap)
  if (p.inBarrel) {
    nA = m_BarrelSectorPerp[p.sector];
    nB = m_BarrelSectorPhi[p.sector];
    nC = TVector3(0.0, 0.0, 1.0);
  } else {
    double out = (p.isForward ? 1.0 : -1.0);
    nA = TVector3(0.0, 0.0, out);
    nB = TVector3(out, 0.0, 0.0);
    nC = TVector3(0.0, out, 0.0);
  }

// Don't adjust the extrapolation if the track is nearly tangent to the readout plane.

  TVector3 extDir = p.momentum.Unit();
  double extDirA = extDir * nA;
  if (fabs(extDirA) < 1.0E-6) { return 1.0E24; }
  double extDirBA = extDir * nB / extDirA;
  double extDirCA = extDir * nC / extDirA;

// Set up the projection jacobian onto the nB-nC measurement plane.

  TMatrixD jacobian(2, 6);
  jacobian[0][0] = nB.X()  - nA.X() * extDirBA;
  jacobian[0][1] = nB.Y()  - nA.Y() * extDirBA;
  jacobian[0][2] = nB.Z()  - nA.Z() * extDirBA;
  jacobian[1][0] = nC.X()  - nA.X() * extDirCA;
  jacobian[1][1] = nC.Y()  - nA.Y() * extDirCA;
  jacobian[1][2] = nC.Z()  - nA.Z() * extDirCA;

// Now get the combined covariance of the extrapolation covariance matrix

  TMatrixDSym correction(p.covariance);  // 6x6
  correction = correction.Similarity(jacobian); // Similarity() changes correction to 2x2!
  return correction[0][0] + correction[1][1];

}

//------------------------------------------------------------------------------
// finished with this track: compute likelihoods and fill the Muid structure

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
  muid->setExtLayerPattern(m_enteredPattern);
  muid->setHitLayerPattern(m_matchedPattern);

  int outcome(0);
  int layerExt(m_lastBarrelLayerExt);
  if (m_lastBarrelLayerExt + m_lastEndcapLayerExt + 1 >= 0) {
    if ((m_firstBarrelLayer > m_LastActiveBarrelLayer) ||
        (m_firstEndcapLayer > m_LastActiveEndcapLayer)) {
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

  m_nPoint += m_nPoint; // 2 independent measurements per detector plane
  double chiSquaredReduced = Large;
  if (m_nPoint > 0) {
    chiSquaredReduced = m_chi2 / m_nPoint;
  }

// Do likelihood calculation

  double junk = 0.0;
  double muon = 0.0;
  double pion = 0.0;
  double kaon = 0.0;
  double proton = 0.0;
  double electron = 0.0;
  double logL_mu = -1.0E200;
  double logL_pi = -1.0E200;
  double logL_K = -1.0E200;
  double logL_p = -1.0E200;
  double logL_e = -1.0E200;
  if (outcome != 0) { // extrapolation reached KLM sensitive volume
    // DIVOT - ignore actual value of chiSquaredReduced for now
    double chiSquaredReduced2 = 0.0;
    if (m_nPoint > 0) {
      for (int kkk = 0; kkk < m_nPoint; ++kkk) {
        double x = gRandom->Gaus(0.0, 1.0);
        chiSquaredReduced2 += x * x;
      }
      chiSquaredReduced2 /= m_nPoint;
    } else {
      chiSquaredReduced2 = 0.0;
    }
    chiSquaredReduced = chiSquaredReduced2;
    // END DIVOT
    muon = m_muonPar->getPDF(outcome, layerExt, layerDiff, chiSquaredReduced);
    pion = m_pionPar->getPDF(outcome, layerExt, layerDiff, chiSquaredReduced);
    kaon = m_kaonPar->getPDF(outcome, layerExt, layerDiff, chiSquaredReduced);
    proton = 0.0; // DIVOT m_protonPar->getPDF(outcome, layerExt, layerDiff, chiSquaredReduced);
    electron = 0.0; // DIVOT m_electronPar->getPDF(outcome, layerExt, layerDiff, chiSquaredReduced);
    logL_mu   = (muon > 0.0 ? log(muon) : -1.0E200);
    logL_pi   = (pion > 0.0 ? log(pion) : -1.0E200);
    logL_K    = (kaon > 0.0 ? log(kaon) : -1.0E200);
    logL_p    = (proton > 0.0 ? log(proton) : -1.0E200);
    logL_e    = (electron > 0.0 ? log(electron) : -1.0E200);
    // now normalize the PDF values for the muon vs *meson* hypotheses
    double denom = muon + 0.5 * (pion + kaon);
    if (denom < 1.0E-200) {
      junk = 1.0; // anomaly: should never happen!!
    } else {
      muon /= denom;
      pion /= denom;
      kaon /= denom;
      proton /= denom;
      electron /= denom;
    }
  }

  muid->setJunkPDFValue(junk);
  muid->setMuonPDFValue(muon);
  muid->setPionPDFValue(pion);
  muid->setKaonPDFValue(kaon);
  muid->setProtonPDFValue(proton);
  muid->setElectronPDFValue(electron);
  muid->setLogL_mu(logL_mu);
  muid->setLogL_pi(logL_pi);
  muid->setLogL_K(logL_K);
  muid->setLogL_K(logL_p);
  muid->setLogL_K(logL_e);

}

TMatrixDSym MuidModule::fromG4eToPhasespace(const G4ErrorFreeTrajState* state)
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
  for (int k = 0; k < 6; ++k) {
    for (int j = 0; j < 6; ++j) {
      covariance[j][k] = phasespaceCov[j][k];
    }
  }

  return covariance;

}

G4ErrorTrajErr MuidModule::fromPhasespaceToG4e(const TVector3& momentum, const TMatrixDSym& covariance)
{

  // Convert phase-space covariance matrix with parameters x, y, z, px, py, pz (in GeV/c, cm)
  // to Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in GeV/c, radians, cm)
  // (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
  // phi = atan( py / px )
  // lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )
  // xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
  // yT = -x * sin(phi) + y * cos(phi)
  // zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)

  G4ErrorSymMatrix temp(6, 0);
  for (int k = 0; k < 6; k++) {
    for (int j = 0; j < 6; j++) {
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

