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
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <genfit/Track.h>
#include <genfit/DetPlane.h>
#include <genfit/FieldManager.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>
#include <genfit/Exception.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <bklm/geometry/GeometryPar.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <TMatrixDSym.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TVector3.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <globals.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4VPhysicalVolume.hh>
// not needed #include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4RegionStore.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
// not needed #include <G4TransportationManager.hh>
// not needed #include <G4FieldManager.hh>
#include <G4UImanager.hh>

using namespace std;
using namespace CLHEP;
using namespace Belle2;

#define TWOPI 6.283185482025146484375
#define M_PI_8 0.3926990926265716552734
#define DEPTH_RPC 9

REG_MODULE(Muid)

MuidModule::MuidModule() :
  Module(),
  m_ExtMgr(NULL),
  m_BKLMVolumes(NULL),
  m_EKLMVolumes(NULL),
  m_TOF(0.0),
  m_MinRadiusSq(0.0),
  m_Target(NULL),
  m_OffsetZ(0.0),
  m_EndcapMaxR(0.0),
  m_EndcapMinR(0.0),
  m_BarrelMinR(0.0),
  m_BarrelMaxR(0.0),
  m_EndcapHalfLength(0.0),
  m_BarrelHalfLength(0.0),
  m_OutermostActiveEndcapLayer(0),
  m_OutermostActiveBarrelLayer(0),
  m_EndcapMiddleZ(0.0),
  m_BarrelScintVariance(0.0),
  m_EndcapScintVariance(0.0),
  m_FirstBarrelLayer(0),
  m_FirstEndcapLayer(0),
  m_ExtLayerPattern(0),
  m_HitLayerPattern(0),
  m_Escaped(false),
  m_Chi2(0.0),
  m_NPoint(0),
  m_LastBarrelExtLayer(0),
  m_LastBarrelHitLayer(0),
  m_LastEndcapExtLayer(0),
  m_LastEndcapHitLayer(0),
  m_MuonPlusPar(NULL),
  m_MuonMinusPar(NULL),
  m_PionPlusPar(NULL),
  m_PionMinusPar(NULL),
  m_KaonPlusPar(NULL),
  m_KaonMinusPar(NULL),
  m_ProtonPar(NULL),
  m_AntiprotonPar(NULL),
  m_DeuteronPar(NULL),
  m_AntideuteronPar(NULL),
  m_ElectronPar(NULL),
  m_PositronPar(NULL)
{
  for (int j = 0; j < NLAYER + 1; ++j) {
    m_BarrelPhiStripVariance[j] = 0.0;
    m_BarrelZStripVariance[j] = 0.0;
    m_BarrelPhiStripVariance[j] = 0.0;
    m_BarrelModuleMiddleRadius[j] = 0.0;
    m_EndcapModuleMiddleZ[j] = 0.0;
  }
  for (int j = 0; j < NSECTOR + 1; ++j) {
    m_BarrelSectorPerp[j] = TVector3(0.0, 0.0, 0.0);
    m_BarrelSectorPhi[j] = TVector3(0.0, 0.0, 0.0);
  }
  m_PDGCode.clear();
  setDescription("Identifies muons by extrapolating tracks from CDC to KLM using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("pdgCodes", m_PDGCode, "Positive-charge PDG codes for extrapolation hypotheses", m_PDGCode);
  addParam("BKLMHitsColName", m_BKLMHitsColName, "Name of collection holding the reconstructed 2D hits in barrel KLM", string(""));
  addParam("EKLMHitsColName", m_EKLMHitsColName, "Name of collection holding the reconstructed 2D hits in endcap KLM", string(""));
  addParam("TracksColName", m_TracksColName, "Name of collection holding the reconstructed tracks", string(""));
  addParam("MuidsColName", m_MuidsColName, "Name of collection holding the muon identification information from the extrapolation",
           string(""));
  addParam("MuidHitsColName", m_MuidHitsColName, "Name of collection holding the muidHits from the extrapolation", string(""));
  addParam("KLMClustersColName", m_KLMClustersColName, "Name of collection holding the KLMClusters", string(""));
  addParam("MinPt", m_MinPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated.", double(0.1));
  addParam("MinKE", m_MinKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation.", double(0.002));
  addParam("MaxStep", m_MaxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity).", double(25.0));
  addParam("MaxDistSigma", m_MaxDistSqInVariances, "[#sigmas] Maximum hit-to-extrapolation difference.", double(7.5));
  addParam("MaxKLMClusterTrackConeAngle", m_MaxClusterTrackConeAngle,
           "[degrees] Maximum cone angle between matching track and KLM cluster.", double(15.0));
  addParam("Cosmic", m_Cosmic, "Particle source (0 = beam, 1 = cosmic ray.", 0);
  // Additional parameters copied from FullSimModule
  addParam("TrackingVerbosity", m_trackingVerbosity,
           "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info.",
           0);
  addParam("EnableVisualization", m_enableVisualization, "If set to True the Geant4 visualization support is enabled.", false);
  addParam("magneticField", m_magneticFieldName,
           "Chooses the magnetic field stepper used by Geant4. possible values are: default, nystrom, expliciteuler, simplerunge",
           string("default"));
  addParam("magneticCacheDistance", m_magneticCacheDistance,
           "Minimum distance for BField lookup in cm. If the next requested point is closer than this distance than return the flast BField value. 0 means no caching",
           0.0);
  addParam("deltaChordInMagneticField", m_deltaChordInMagneticField,
           "[mm] The maximum miss-distance between the trajectory curve and its linear cord(s) approximation", 0.25);
  vector<string> defaultCommands;
  addParam("UICommands", m_uiCommands, "A list of Geant4 UI commands that should be applied at the start of the job.",
           defaultCommands);
}

MuidModule::~MuidModule()
{
}

void MuidModule::initialize()
{

  // Convert from GeV to GEANT4 energy units (MeV); avoid negative values
  m_MinPt = max(0.0, m_MinPt) * GeV;
  m_MinKE = max(0.0, m_MinKE) * GeV;

  // Convert user's maximum track-KLMCluster cone angle from degrees to radians
  m_MaxClusterTrackConeAngle *= M_PI / 180.0;

  // Define the list of BKLM/EKLM sensitive volumes in the geant4 geometry
  registerVolumes();

  // Initialize the (singleton) extrapolation manager.  It will check
  // whether it will run with or without FullSimModule and with or without
  // Ext (or any other geant4e-based extrapolation module).
  m_ExtMgr = Simulation::ExtManager::GetManager();
  m_ExtMgr->Initialize("Muid", m_magneticFieldName, m_magneticCacheDistance,
                       m_deltaChordInMagneticField,
                       m_enableVisualization, m_trackingVerbosity,
                       m_uiCommands);

  // Redefine muid's step length, magnetic field step limitation (fraction of local curvature radius),
  // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
  // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
  // *NOTE* If module ext runs after this, its G4UImanager commands will override these.
  G4double maxStep = ((m_MaxStep == 0.0) ? 10.0 : std::min(10.0, m_MaxStep)) * cm;
  char line[80];
  std::sprintf(line, "/geant4e/limits/stepLength %8.2f mm", maxStep);
  G4UImanager::GetUIpointer()->ApplyCommand(line);
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
  G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");

  GearDir strContent = GearDir("Detector/DetectorComponent[@name=\"COIL\"]/Content/");
  double rMaxCoil = strContent.getLength("Cryostat/Rmin") * cm;
  m_MinRadiusSq = (rMaxCoil * 0.25) * (rMaxCoil * 0.25); // roughly 40 cm

  GearDir bklmContent = GearDir("/Detector/DetectorComponent[@name=\"BKLM\"]/Content/");
  GearDir eklmContent = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/");
  m_BarrelHalfLength = bklmContent.getLength("HalfLength") * cm; // in G4 units (mm)
  m_EndcapHalfLength = 0.5 * eklmContent.getLength("Endcap/Length") * cm; // in G4 units (mm)
  m_OffsetZ = bklmContent.getLength("OffsetZ") * cm; // in G4 units (mm)
  double minZ = m_OffsetZ - (m_BarrelHalfLength + 2.0 * m_EndcapHalfLength);
  double maxZ = m_OffsetZ + (m_BarrelHalfLength + 2.0 * m_EndcapHalfLength);
  m_BarrelMaxR = bklmContent.getLength("OuterRadius") * cm / cos(M_PI / bklmContent.getNumberNodes("Sectors/Forward/Sector"));
  m_Target = new Simulation::ExtCylSurfaceTarget(m_BarrelMaxR, minZ, maxZ);
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_Target);

  m_BarrelHalfLength /= cm;  // in G4e units (cm)
  m_EndcapHalfLength /= cm;  // in G4e units (cm)
  m_OffsetZ /= cm;           // in G4e units (cm)
  m_BarrelMinR = bklmContent.getLength("Layers/InnerRadius");  // in G4e units (cm)
  m_BarrelMaxR /= cm;                                          // in G4e units (cm)
  m_EndcapMinR = eklmContent.getLength("Endcap/InnerR");       // in G4e units (cm)
  m_EndcapMaxR = eklmContent.getLength("Endcap/OuterR");       // in G4e units (cm)
  m_EndcapMiddleZ = m_BarrelHalfLength + m_EndcapHalfLength;   // in G4e units (cm)

  // Measurement uncertainties and acceptance windows
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

  m_OutermostActiveBarrelLayer = nBarrelLayers - 1; // zero-based counting
  for (int layer = 1; layer <= nBarrelLayers; ++layer) {
    m_BarrelModuleMiddleRadius[layer - 1] = bklm::GeometryPar::instance()->getActiveMiddleRadius(layer); // in G4e units (cm)
  }
  double dz(eklmContent.getLength("Endcap/Layer/ShiftZ")); // in G4e units (cm)
//  double z0(eklmContent.getLength("Endcap/PositionZ") - m_OffsetZ + dz
//            - 0.5 * eklmContent.getLength("Endcap/Layer/Length")
//            - 0.5 * eklmContent.getLength("Endcap/Layer/Sector/Plane/Strips/Thickness")
//            - 0.5 * eklmContent.getLength("Endcap/Layer/Sector/Plane/PlasticSheetWidth")); // in G4e units (cm)
  double z0(eklmContent.getLength("Endcap/PositionZ")
            - 0.5 * eklmContent.getLength("Endcap/Length")
            + dz - 0.5 * eklmContent.getLength("Endcap/Layer/Length"));

  int nEndcapLayers = eklmContent.getInt("Endcap/nLayer");
  m_OutermostActiveEndcapLayer = nEndcapLayers - 1; // zero-based counting
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
  if (m_PDGCode.empty()) {
    m_ChargedStable.push_back(Const::muon);
  } else { // user defined
    const Const::ParticleSet set = Const::chargedStableSet;
    std::vector<Const::ChargedStable> stack;
    for (const Const::ChargedStable& pdgIter : set) {
      stack.push_back(pdgIter);
    }
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
    B2INFO("Muid hypothesis for PDG code " << m_ChargedStable[i].getPDGCode() << " and its antiparticle will be extrapolated")
  }

  // Register output and relation arrays' persistence
  StoreArray<Track> tracks(m_TracksColName);
  StoreArray<Muid> muids(m_MuidsColName);
  StoreArray<MuidHit> muidHits(m_MuidHitsColName);
  StoreArray<KLMCluster> klmClusters(m_KLMClustersColName);
  muids.registerInDataStore();
  muidHits.registerInDataStore();
  klmClusters.registerInDataStore();
  tracks.registerRelationTo(muids);
  tracks.registerRelationTo(muidHits);
  tracks.registerRelationTo(klmClusters);

  return;

}

void MuidModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  int expNo = evtMetaData->getExperiment();
  B2INFO("muid: Experiment " << expNo << "  run " << evtMetaData->getRun())
  m_MuonPlusPar = new MuidPar(expNo, "MuonPlus");
  m_MuonMinusPar = new MuidPar(expNo, "MuonMinus");
  m_PionPlusPar = new MuidPar(expNo, "PionPlus");
  m_PionMinusPar = new MuidPar(expNo, "PionMinus");
  m_KaonPlusPar = new MuidPar(expNo, "KaonPlus");
  m_KaonMinusPar = new MuidPar(expNo, "KaonMinus");
  m_ProtonPar = new MuidPar(expNo, "Proton");
  m_AntiprotonPar = new MuidPar(expNo, "Antiproton");
  m_DeuteronPar = new MuidPar(expNo, "Deuteron");
  m_AntideuteronPar = new MuidPar(expNo, "Antideuteron");
  m_ElectronPar = new MuidPar(expNo, "Electron");
  m_PositronPar = new MuidPar(expNo, "Positron");
}

void MuidModule::event()
{

  // Put geant4 in proper state (in case this module is in a separate process)
  if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  // Loop over the reconstructed tracks.
  // Do extrapolation for each hypotheses of each reconstructed track.

  StoreArray<Track> tracks(m_TracksColName);
  StoreArray<Muid> muids(m_MuidsColName);
  StoreArray<KLMCluster> klmClusters(m_KLMClustersColName);

  G4Point3D position;
  G4Vector3D momentum;
  G4ErrorTrajErr covG4e(5, 0);

  for (int t = 0; t < tracks.getEntries(); ++t) {

    // Typically, only the muon hypothesis is used.  Others are for debugging.
    for (unsigned int hypothesis = 0; hypothesis < m_ChargedStable.size(); ++hypothesis) {

      Const::ChargedStable chargedStable = m_ChargedStable[hypothesis];
      int pdgCode = chargedStable.getPDGCode();
      if (chargedStable == Const::electron || chargedStable == Const::muon) pdgCode = -pdgCode;

      Muid* muid = muids.appendNew(pdgCode); // pdgCode doesn't know charge yet
      tracks[t]->addRelationTo(muid);

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

      pdgCode *= int(gfTrack->getFittedState().getCharge());
      muid->setPDGCode(pdgCode);
      getStartPoint(gfTrack, pdgCode, position, momentum, covG4e);
      muid->setMomentum(momentum.x(), momentum.y(), momentum.z());
      if (momentum.perp() <= m_MinPt) continue;
      if (m_Target->GetDistanceFromPoint(position) < 0.0) continue;
      G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
      string g4eName = "g4e_" + particle->GetParticleName();
      double mass = particle->GetPDGMass();
      double minPSq = (mass + m_MinKE) * (mass + m_MinKE) - mass * mass;
      G4ErrorFreeTrajState* state = new G4ErrorFreeTrajState(g4eName, position, momentum, covG4e);
      m_ExtMgr->InitTrackPropagation();
      while (true) {

        const G4int    errCode    = m_ExtMgr->PropagateOneStep(state);
        G4Track*       track      = state->GetG4Track();
        const G4Step*  step       = track->GetStep();
        // Ignore the zero-length step by PropagateOneStep() at each boundary
        if (step->GetStepLength() > 0.0) {
          m_TOF += step->GetDeltaTime();
          if (createHit(state, tracks[t], pdgCode)) {
            // Force geant4e to update its G4Track from the Kalman-updated state
            m_ExtMgr->GetPropagator()->SetStepN(0);
          }
        }
        // Detect radial escapes from the EKLM.  (Longitudinal escapes from
        // EKLM and BKLM are detected elsewhere.)
        if (m_Target->GetDistanceFromPoint(track->GetPosition()) < 0.0) m_Escaped = true;
        // Stop extrapolating as soon as the track escapes KLM or curls inward too much
        // or the momentum is too low
        if (m_Escaped) break;
        if (track->GetPosition().perp2() < m_MinRadiusSq) break;
        if (track->GetMomentum().mag2() < minPSq) break;
        if (errCode) break;

      } // track-extrapolation "infinite" loop

      m_ExtMgr->EventTermination();

      delete state;

      finishTrack(muid, int(gfTrack->getFittedState().getCharge()));

      // Find the matching KLMCluster(s)
      for (int c = 0; c < klmClusters.getEntries(); ++c) {
        G4Point3D clusterDirection(klmClusters[c]->getPosition().X(),
                                   klmClusters[c]->getPosition().Y(),
                                   klmClusters[c]->getPosition().Z());
        if (position.angle(clusterDirection) < m_MaxClusterTrackConeAngle) {
          klmClusters[c]->setAssociatedTrackFlag();
          tracks[t]->addRelationTo(klmClusters[c]);
          break;
        }
      }

    } // hypothesis loop

  } // track loop

}

void MuidModule::endRun()
{
}

void MuidModule::terminate()
{

  delete m_Target;
  delete m_BKLMVolumes;
  delete m_EKLMVolumes;
  m_ExtMgr->RunTermination();

  delete m_MuonPlusPar;
  delete m_MuonMinusPar;
  delete m_PionPlusPar;
  delete m_PionMinusPar;
  delete m_KaonPlusPar;
  delete m_KaonMinusPar;
  delete m_ProtonPar;
  delete m_AntiprotonPar;
  delete m_DeuteronPar;
  delete m_AntideuteronPar;
  delete m_ElectronPar;
  delete m_PositronPar;

}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void MuidModule::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("No geometry defined. Please create the geometry first.")
  }

  m_BKLMVolumes = new vector<G4VPhysicalVolume*>;
  m_EKLMVolumes = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // see belle2/run/volname3.txt:
    // Barrel KLM: BKLM.Layer**GasPhysical for RPCs or BKLM.Layer**ChimneyGasPhysical for RPCs
    //             BKLM.ScintType*Physical for scintillator strips
    if (name.substr(0, 5) == "BKLM.") {
      if ((name.find("ScintType") != string::npos) ||
          (name.find("GasPhysical") != string::npos)) {
        m_BKLMVolumes->push_back(*iVol);
      }
    }
    // Endcap KLM:
    // Sensitive_Strip_StripVolume_{1..75}_Plane_{1,2}_Sector_{1..4}_Layer_{1..14}_Endcap_{1,2}
    if (name.substr(0, 27) == "Sensitive_Strip_StripVolume") {
      m_EKLMVolumes->push_back(*iVol);
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
    // Do not extrapolate this track by forcing minPt cut to fail in caller
    momentum.setX(0.0);
    momentum.setY(0.0);
    momentum.setZ(0.0);
  }

  // Keep track of geometrical state during one track's extrapolation
  m_FirstBarrelLayer = 0;   // ratchets outward when looking for matching barrel hits
  m_FirstEndcapLayer = 0;   // ratchets outward when looking for matching endcap hits
  m_Escaped = false;

  // Quantities that will be written to StoreArray<Muid> at end of track
  m_ExtLayerPattern = 0x00000000;
  m_HitLayerPattern = 0x00000000;
  m_Chi2 = 0.0;
  m_NPoint = 0;
  m_LastBarrelExtLayer = -1;    // track hasn't crossed a barrel layer yet
  m_LastBarrelHitLayer = -1;    // track hasn't crossed an endcap layer yet
  m_LastEndcapExtLayer = -1;    // no matching hit in barrel layer yet
  m_LastEndcapHitLayer = -1;    // no matching hit in endcap layer yet

  return;

}

// Write another volume-entry point on track.
// The track state will be modified here by the Kalman fitter.

bool MuidModule::createHit(G4ErrorFreeTrajState* state, Track* track, int pdgCode)
{

  Point point;
  point.chi2 = -1.0;
  point.position.SetX(state->GetPosition().x() / cm);
  point.position.SetY(state->GetPosition().y() / cm);
  point.position.SetZ(state->GetPosition().z() / cm);
  point.momentum.SetX(state->GetMomentum().x() / GeV);
  point.momentum.SetY(state->GetMomentum().y() / GeV);
  point.momentum.SetZ(state->GetMomentum().z() / GeV);
  G4ThreeVector prePos = state->GetG4Track()->GetStep()->GetPreStepPoint()->GetPosition() / cm;
  TVector3 oldPosition(prePos.x(), prePos.y(), prePos.z());
  double r = point.position.Perp();
  double z = fabs(point.position.Z() - m_OffsetZ);

  // Is the track in the barrel?
  if ((r > m_BarrelMinR) && (r < m_BarrelMaxR) && (z < m_BarrelHalfLength)) {
    // Did the track cross the inner midplane of a detector module?
    if (findBarrelIntersection(oldPosition, point)) {
      point.covariance.ResizeTo(6, 6);
      point.covariance = fromG4eToPhasespace(state);
      if (findMatchingBarrelHit(point)) {
        m_ExtLayerPattern |= (0x00000001 << point.layer);
        if (m_LastBarrelExtLayer < point.layer) {
          m_LastBarrelExtLayer = point.layer;
        }
        m_HitLayerPattern |= (0x00000001 << point.layer);
        if (m_LastBarrelHitLayer < point.layer) {
          m_LastBarrelHitLayer = point.layer;
        }
        // If the updated point is outside the barrel, discard it and the Kalman-fitter adjustment
        r = point.position.Perp();
        z = fabs(point.position.Z() - m_OffsetZ);
        if ((r <= m_BarrelMinR) || (r >= m_BarrelMaxR) || (z >= m_BarrelHalfLength)) {
          point.chi2 = -1.0;
        }
      } else {
        // Record a no-hit track crossing if this step is strictly within a barrel sensitive volume
        vector<G4VPhysicalVolume*>::iterator j = find(m_BKLMVolumes->begin(), m_BKLMVolumes->end(), state->GetG4Track()->GetVolume());
        if (j != m_BKLMVolumes->end()) {
          m_ExtLayerPattern |= (0x00000001 << point.layer);
          if (m_LastBarrelExtLayer < point.layer) {
            m_LastBarrelExtLayer = point.layer;
          }
        }
      }
    }
  }

  // Is the track in the endcap?
  if ((r > m_EndcapMinR) && (fabs(z - m_EndcapMiddleZ) < m_EndcapHalfLength)) {
    // Did the track cross the inner midplane of a detector module?
    if (findEndcapIntersection(oldPosition, point)) {
      point.covariance.ResizeTo(6, 6);
      point.covariance = fromG4eToPhasespace(state);
      if (findMatchingEndcapHit(point)) {
        m_ExtLayerPattern |= (0x00008000 << point.layer);
        if (m_LastEndcapExtLayer < point.layer) {
          m_LastEndcapExtLayer = point.layer;
        }
        m_HitLayerPattern |= (0x00008000 << point.layer);
        if (m_LastEndcapHitLayer < point.layer) {
          m_LastEndcapHitLayer = point.layer;
        }
        // If the updated point is outside the endcap, discard it and the Kalman-fitter adjustment
        r = point.position.Perp();
        z = fabs(point.position.Z() - m_OffsetZ);
        if ((r <= m_EndcapMinR) || (r >= m_EndcapMaxR) || (fabs(z - m_EndcapMiddleZ) >= m_EndcapHalfLength)) {
          point.chi2 = -1.0;
        }
      } else {
        // Record a no-hit track crossing if this step is strictly within an endcap sensitive volume
        vector<G4VPhysicalVolume*>::iterator j = find(m_EKLMVolumes->begin(), m_EKLMVolumes->end(), state->GetG4Track()->GetVolume());
        if (j != m_EKLMVolumes->end()) {
          m_ExtLayerPattern |= (0x00008000 << point.layer);
          if (m_LastEndcapExtLayer < point.layer) {
            m_LastEndcapExtLayer = point.layer;
          }
        }
      }
    }
  }

  // Create a new MuidHit and RelationEntry between it and the track.
  // Adjust geant4e's position, momentum and covariance based on matching hit and tell caller to update the geant4e state.
  if (point.chi2 >= 0.0) {
    StoreArray<MuidHit> muidHits(m_MuidHitsColName);
    MuidHit* muidHit = muidHits.appendNew(pdgCode, point.inBarrel, point.isForward, point.sector, point.layer, point.position,
                                          point.positionAtHitPlane, m_TOF, point.time, point.chi2);
    track->addRelationTo(muidHit);
    G4Point3D newPos(point.position.X()*cm, point.position.Y()*cm, point.position.Z()*cm);
    state->SetPosition(newPos);
    G4Vector3D newMom(point.momentum.X()*GeV, point.momentum.Y()*GeV, point.momentum.Z()*GeV);
    state->SetMomentum(newMom);
    state->SetError(fromPhasespaceToG4e(point.momentum, point.covariance));
    m_Chi2 += point.chi2;
    m_NPoint += 2; // two (orthogonal) independent hits per detector layer
    return true;
  }

  // Tell caller that the geant4e state was not modified.
  return false;

}

bool MuidModule::findBarrelIntersection(const TVector3& oldPosition, Point& point)
{

  // Be generous: allow outward-moving intersection to be in the dead space between
  // largest sensitive-volume Z and m_BarrelHalfLength, not necessarily in a geant4 sensitive volume

  if (fabs(point.position.Z() - m_OffsetZ) > m_BarrelHalfLength) return false;

  double phi = point.position.Phi();
  if (phi < 0.0) { phi += TWOPI; }
  if (phi > TWOPI - M_PI_8) { phi -= TWOPI; }
  int sector = (int)((phi + M_PI_8) / M_PI_4);

  double oldR = oldPosition * m_BarrelSectorPerp[sector];
  double newR = point.position * m_BarrelSectorPerp[sector];

  for (int layer = m_FirstBarrelLayer; layer <= m_OutermostActiveBarrelLayer; ++layer) {
    if (newR <  m_BarrelModuleMiddleRadius[layer]) break;
    if (oldR <= m_BarrelModuleMiddleRadius[layer]) {
      m_FirstBarrelLayer = layer + 1; // ratchet outward for next call's loop starting value
      if (m_FirstBarrelLayer > m_OutermostActiveBarrelLayer) m_Escaped = true;
      point.inBarrel = true;
      point.isForward = point.position.Z() > m_OffsetZ;
      point.layer = layer;
      point.sector = sector;
      return true;
    }
  }

  return false;

}

bool MuidModule::findEndcapIntersection(const TVector3& oldPosition, Point& point)
{

  // Be generous: allow intersection to be in the dead space between m_EndcapMinR and innermost
  // sensitive-volume radius or between outermost sensitive-volume radius and m_EndcapMaxR,
  // not necessarily in a geant4 sensitive volume

  if (oldPosition.Perp() > m_EndcapMaxR) return false;
  if (point.position.Perp() < m_EndcapMinR) return false;

  double oldZ = fabs(oldPosition.Z() - m_OffsetZ);
  double newZ = fabs(point.position.Z() - m_OffsetZ);

  for (int layer = m_FirstEndcapLayer; layer <= m_OutermostActiveEndcapLayer; ++layer) {
    if (newZ <  m_EndcapModuleMiddleZ[layer]) break;
    if (oldZ <= m_EndcapModuleMiddleZ[layer]) {
      m_FirstEndcapLayer = layer + 1; // ratchet outward for next call's loop starting value
      if (m_FirstEndcapLayer > m_OutermostActiveEndcapLayer) m_Escaped = true;
      point.inBarrel = false;
      point.isForward = point.position.Z() > m_OffsetZ;
      point.layer = layer;
      double phi = point.position.Phi();
      if (phi < 0.0) { phi += TWOPI; }
      if (point.isForward) {
        phi = M_PI - phi;
        if (phi < 0.0) { phi += TWOPI; }
      }
      point.sector = (int)(phi / M_PI_2); // my calculation; matches EKLM-geometry calculation
      return true;
    }
  }

  return false;

}

bool MuidModule::findMatchingBarrelHit(Point& point)

{

  TVector3 extPos0(point.position);

  StoreArray<BKLMHit2d> bklmHits(m_BKLMHitsColName);
  double diffBestMagSq = 1.0E60;
  int bestHit = -1;
  int matchingLayer = point.layer + 1;
  TVector3 n(m_BarrelSectorPerp[point.sector]);
  for (int h = 0; h < bklmHits.getEntries(); ++h) {
    BKLMHit2d* hit = bklmHits[h];
    if (hit->getLayer() != matchingLayer) continue;
    if (hit->isOutOfTime()) continue;
    if (hit->getTime() > 50.0) continue; // DIVOT this needs tuning work
    TVector3 diff(hit->getGlobalPosition() - point.position);
    double dn = diff * n; // in cm
    if (fabs(dn) < 2.0) {
      // Hit and extrapolated point are in the same sector
      diff -= n * dn;
      if (diff.Mag2() < diffBestMagSq) {
        diffBestMagSq = diff.Mag2();
        bestHit = h;
        extPos0 = point.position;
      }
    } else {
      // Accept a nearby hit in adjacent sector
      if (fabs(dn) > 50.0) continue;
      int dSector = abs(point.sector - hit->getSector() + 1);
      if ((dSector != 1) && (dSector != 7)) continue;
      // Use the normal vector of the adjacent (hit's) sector
      TVector3 nHit(m_BarrelSectorPerp[hit->getSector() - 1]);
      double dn2 = point.position * nHit - m_BarrelModuleMiddleRadius[point.layer];
      dn = diff * nHit + dn2;
      if (fabs(dn) > 1.0) continue;
      // Project extrapolated track to the hit's plane in the adjacent sector
      TVector3 extDir(point.momentum.Unit());
      double extDirA = extDir * nHit;
      if (fabs(extDirA) < 1.0E-6) continue;
      TVector3 projection = extDir * (dn2 / extDirA);
      if (projection.Mag() > 15.0) continue;
      diff += projection - nHit * dn;
      if (diff.Mag2() < diffBestMagSq) {
        diffBestMagSq = diff.Mag2();
        bestHit = h;
        extPos0 = point.position - projection;
      }
    }
  }

  if (bestHit >= 0) {
    BKLMHit2d* hit = bklmHits[bestHit];
    point.isForward = hit->isForward();
    point.sector = hit->getSector() - 1;
    point.time = hit->getTime();
    double localVariance[2] = {m_BarrelScintVariance, m_BarrelScintVariance};
    if (hit->inRPC()) {
      localVariance[0] = m_BarrelPhiStripVariance[point.layer];
      localVariance[1] = m_BarrelZStripVariance[point.layer];
    }
    adjustIntersection(point, localVariance, hit->getGlobalPosition(), extPos0);
    if (point.chi2 >= 0.0) hit->isOnTrack(true);
  }
  return point.chi2 >= 0.0;

}

bool MuidModule::findMatchingEndcapHit(Point& point)
{

  StoreArray<EKLMHit2d> eklmHits(m_EKLMHitsColName);
  double diffBestMagSq = 1.0E60;
  int bestHit = -1;
  int matchingLayer = point.layer + 1;
  TVector3 n(0.0, 0.0, (point.isForward ? 1.0 : -1.0));
  for (int h = 0; h < eklmHits.getEntries(); ++h) {
    EKLMHit2d* hit = eklmHits[h];
    if (hit->getLayer() != matchingLayer) continue;
    if (point.isForward) {
      if (hit->getEndcap() != 2) continue;
    } else {
      if (hit->getEndcap() != 1) continue;
    }
    // DIVOT no such function! if (hit->isOutOfTime()) continue;
    if (hit->getTime() > 50.0) continue; // DIVOT this needs tuning work
    TVector3 diff(hit->getPosition() - point.position);
    double dn = diff * n; // in cm
    if (fabs(dn) > 2.0) continue;
    diff -= n * dn;
    if (diff.Mag2() < diffBestMagSq) {
      diffBestMagSq = diff.Mag2();
      bestHit = h;
    }
  }

  if (bestHit >= 0) {
    EKLMHit2d* hit = eklmHits[bestHit];
    point.isForward = (hit->getEndcap() == 2);
    point.sector = hit->getSector() - 1;
    point.time = hit->getTime();
    double localVariance[2] = {m_EndcapScintVariance, m_EndcapScintVariance};
    adjustIntersection(point, localVariance, hit->getPosition(), point.position);
    // DIVOT no such function! if (point.chi2 >= 0.0) hit->isOnTrack(true);
  }
  return point.chi2 >= 0.0;

}

void MuidModule::adjustIntersection(Point& point, const double localVariance[2], const TVector3& hitPos, const TVector3& extPos0)
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

// In most cases, extPos0 is the same as point.position.  They differ only when
// the nearest BKLM hit is in the sector adjacent to that of point.position.

  TVector3 extPos(extPos0);
  TVector3 extMom(point.momentum);
  TVector3 extDir(extMom.Unit());
  TVector3 diffPos(hitPos - extPos);
  TMatrixDSym extCov(point.covariance);

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
  if (point.inBarrel) {
    nA = m_BarrelSectorPerp[point.sector];
    nB = m_BarrelSectorPhi[point.sector];
    nC = TVector3(0.0, 0.0, 1.0);
  } else {
    double out = (point.isForward ? 1.0 : -1.0);
    nA = TVector3(0.0, 0.0, out);
    nB = TVector3(out, 0.0, 0.0);
    nC = TVector3(0.0, out, 0.0);
  }

// Don't adjust the extrapolation if the track is nearly tangent to the readout plane.

  double extDirA = extDir * nA;
  if (fabs(extDirA) < 1.0E-6) return;
  double extDirBA = extDir * nB / extDirA;
  double extDirCA = extDir * nC / extDirA;

// Move the extrapolated coordinate (at most a tiny amount!) to the plane of the hit.
// If the moved point is outside the KLM, don't do Kalman filtering.

  TVector3 move = extDir * ((diffPos * nA) / extDirA);
  extPos += move;
  diffPos -= move;
  point.positionAtHitPlane.SetX(extPos.X());
  point.positionAtHitPlane.SetY(extPos.Y());
  point.positionAtHitPlane.SetZ(extPos.Z());

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

// Ignore the best hit if it is too far from the extrapolated-track intersection in the hit's plane

  if (residual[0] * residual[0] > correction[0][0] * m_MaxDistSqInVariances) return;
  if (residual[1] * residual[1] > correction[1][1] * m_MaxDistSqInVariances) return;

  double determinant = 0.0;
  correction.Invert(&determinant);
  if (determinant == 0.0) return;

// Matrix inversion succeeeded and is reasonable.
// Evaluate chi-squared increment assuming that the Kalman filter
// won't be able to adjust the extrapolated track's position (fall-back).

  point.chi2 = correction.Similarity(residual);

// Do the Kalman filtering

  TMatrixD gain(6, 2);
  gain.MultT(extCov, jacobian);
  gain *= correction;
  TMatrixDSym HRH(correction.SimilarityT(jacobian));

  extCov -= HRH.Similarity(extCov);
  extPar += gain * residual;
  extPos.SetX(extPar[0]);
  extPos.SetY(extPar[1]);
  extPos.SetZ(extPar[2]);
  extMom.SetX(extPar[3]);
  extMom.SetY(extPar[4]);
  extMom.SetZ(extPar[5]);

// Calculate the chi-squared increment using the Kalman-filtered state

  correction = extCov;
  correction = hitCov - correction.Similarity(jacobian);
  correction.Invert(&determinant);
  if (determinant == 0.0) return;

  diffPos = hitPos - extPos;
  residual[0] = diffPos.X() * jacobian[0][0] + diffPos.Y() * jacobian[0][1] + diffPos.Z() * jacobian[0][2];
  residual[1] = diffPos.X() * jacobian[1][0] + diffPos.Y() * jacobian[1][1] + diffPos.Z() * jacobian[1][2];
  point.chi2 = correction.Similarity(residual);

// Update the position, momentum and covariance of the point
// Project the corrected extrapolation to the plane of the original
// extrapolation's point.position. (Note: point.position is the same as
// extPos0 in all cases except when nearest BKLM hit is in adjacent
// sector, for which extPos0 is a projected position to the hit's plane.)
// Also, leave the momentum magnitude unchanged.

  point.position = extPos + extDir * (((point.position - extPos) * nA) / extDirA);
  point.momentum = point.momentum.Mag() * extMom.Unit();
  point.covariance = extCov;

}

void MuidModule::finishTrack(Muid* muid, int charge)
{

  // Done with this track: compute likelihoods and fill the muid object

  int outcome(0);
  int layerExt(m_LastBarrelExtLayer);
  if ((m_LastBarrelExtLayer >= 0) || (m_LastEndcapExtLayer >= 0)) {
    if (m_Escaped) {
      outcome = 3;
      if (m_LastEndcapExtLayer >= 0) {
        outcome = 4;
        layerExt += m_LastEndcapExtLayer + 1;
      }
    } else {
      outcome = 1;
      if (m_LastEndcapExtLayer >= 0) {
        outcome = 2;
        layerExt += m_LastEndcapExtLayer + 1;
      }
    }
  }

  muid->setOutcome(outcome);
  muid->setBarrelExtLayer(m_LastBarrelExtLayer);
  muid->setEndcapExtLayer(m_LastEndcapExtLayer);
  muid->setBarrelHitLayer(m_LastBarrelHitLayer);
  muid->setEndcapHitLayer(m_LastEndcapHitLayer);
  muid->setExtLayer(m_LastBarrelExtLayer + m_LastEndcapExtLayer + 1);
  muid->setHitLayer((m_LastEndcapHitLayer == -1 ?
                     m_LastBarrelHitLayer :
                     m_LastBarrelExtLayer + m_LastEndcapHitLayer + 1));
  muid->setChiSquared(m_Chi2);
  muid->setDegreesOfFreedom(m_NPoint);
  muid->setExtLayerPattern(m_ExtLayerPattern);
  muid->setHitLayerPattern(m_HitLayerPattern);

  int layerDiff = muid->getExtLayer() - muid->getHitLayer();

// Do likelihood calculation

  double junk = 0.0;
  double muon = 0.0;
  double pion = 0.0;
  double kaon = 0.0;
  double proton = 0.0;
  double deuteron = 0.0;
  double electron = 0.0;
  double logL_mu = -1.0E20;
  double logL_pi = -1.0E20;
  double logL_K = -1.0E20;
  double logL_p = -1.0E20;
  double logL_d = -1.0E20;
  double logL_e = -1.0E20;
  if (outcome != 0) { // extrapolation reached KLM sensitive volume
    if (charge > 0.0) {
      muon = m_MuonPlusPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      pion = m_PionPlusPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      kaon = m_KaonPlusPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      proton = m_ProtonPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      deuteron = m_DeuteronPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      electron = m_PositronPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
    } else {
      muon = m_MuonMinusPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      pion = m_PionMinusPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      kaon = m_KaonMinusPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      proton = m_AntiprotonPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      deuteron = m_AntideuteronPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
      electron = m_ElectronPar->getPDF(outcome, layerExt, layerDiff, m_NPoint, m_Chi2);
    }
    if (muon > 0.0) logL_mu = log(muon);
    if (pion > 0.0) logL_pi = log(pion);
    if (kaon > 0.0) logL_K = log(kaon);
    if (proton > 0.0) logL_p = log(proton);
    if (deuteron > 0.0) logL_d = log(deuteron);
    if (electron > 0.0) logL_e = log(electron);
    // normalize the PDF values
    double denom = muon + pion + kaon + proton + deuteron + electron;
    if (denom < 1.0E-20) {
      junk = 1.0; // anomaly: should be very rare
    } else {
      muon /= denom;
      pion /= denom;
      kaon /= denom;
      proton /= denom;
      deuteron /= denom;
      electron /= denom;
    }
  }

  muid->setJunkPDFValue(junk);
  muid->setMuonPDFValue(muon);
  muid->setPionPDFValue(pion);
  muid->setKaonPDFValue(kaon);
  muid->setProtonPDFValue(proton);
  muid->setDeuteronPDFValue(deuteron);
  muid->setElectronPDFValue(electron);
  muid->setLogL_mu(logL_mu);
  muid->setLogL_pi(logL_pi);
  muid->setLogL_K(logL_K);
  muid->setLogL_p(logL_p);
  muid->setLogL_d(logL_d);
  muid->setLogL_e(logL_e);

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

