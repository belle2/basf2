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
#include <tracking/trackExtrapolateG4e/MuidPar.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/geometry/BFieldManager.h>
#include <bklm/dataobjects/BKLMStatus.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/Muid.h>
#include <tracking/dataobjects/MuidHit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <bklm/geometry/GeometryPar.h>
#include <bklm/geometry/Module.h>
#include <eklm/geometry/GeometryData.h>
#include <genfit/Exception.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Matrix/Vector.h>

#include <globals.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Track.hh>
#include <G4Step.hh>
#include <G4StepPoint.hh>
#include <G4VTouchable.hh>
#include <G4TouchableHandle.hh>
#include <G4NavigationHistory.hh>
#include <G4ParticleTable.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4ThreeVector.hh>
#include <G4ErrorMatrix.hh>
#include <G4ErrorSymMatrix.hh>
#include <G4Point3D.hh>
#include <G4UImanager.hh>

#define TWOPI (2.0*M_PI)
#define PI_8 (0.125*M_PI)
#define DEPTH_RPC 9
#define DEPTH_SCINT 11

using namespace std;
using namespace Belle2;

TrackExtrapolateG4e* TrackExtrapolateG4e::m_Singleton = NULL;

TrackExtrapolateG4e* TrackExtrapolateG4e::getInstance()
{
  if (m_Singleton == NULL) m_Singleton = new TrackExtrapolateG4e;
  return m_Singleton;
}

TrackExtrapolateG4e::TrackExtrapolateG4e() :
  m_ExtInitialized(false), // initialized later
  m_MuidInitialized(false), // initialized later
  m_MeanDt(0.0), // initialized later
  m_MaxDt(0.0), // initialized later
  m_MagneticField(0.0), // initialized later
  m_MaxDistSqInVariances(0.0), // initialized later
  m_MaxKLMTrackClusterDistance(0.0), // initialized later
  m_MaxECLTrackClusterDistance(0.0), // initialized later
  m_MinPt(0.0), // initialized later
  m_MinKE(0.0), // initialized later
  m_TracksColName(NULL), // initialized later
  m_ExtHitsColName(NULL), // initialized later
  m_MuidsColName(NULL), // initialized later
  m_MuidHitsColName(NULL), // initialized later
  m_BKLMHitsColName(NULL), // initialized later
  m_EKLMHitsColName(NULL), // initialized later
  m_KLMClustersColName(NULL), // initialized later
  m_ECLClustersColName(NULL), // initialized later
  m_TrackClusterSeparationsColName(NULL), // initialized later
  m_ExtMgr(NULL), // initialized later
  m_HypothesesExt(NULL), // initialized later
  m_HypothesesMuid(NULL), // initialized later
  m_DefaultName(NULL), // initialized later
  m_DefaultHypotheses(NULL), // initialized later
  m_EnterExit(NULL), // initialized later
  m_BKLMVolumes(NULL), // initialized later
  m_EKLMVolumes(NULL), // initialized later
  m_TargetExt(NULL), // initialized later
  m_TargetMuid(NULL), // initialized later
  m_MinRadiusSq(0.0), // initialized later
  m_OffsetZ(0.0), // initialized later
  m_BarrelMaxR(0.0), // initialized later
  m_BarrelMinR(0.0), // initialized later
  m_BarrelHalfLength(0.0), // initialized later
  m_OutermostActiveBarrelLayer(0), // initialized later
  m_BarrelScintVariance(0.0), // initialized later
  m_EndcapMaxR(0.0), // initialized later
  m_EndcapMinR(0.0), // initialized later
  m_EndcapMiddleZ(0.0), // initialized later
  m_EndcapHalfLength(0.0), // initialized later
  m_OutermostActiveForwardEndcapLayer(0), // initialized later
  m_OutermostActiveBackwardEndcapLayer(0), // initialized later
  m_EndcapScintVariance(0.0), // initialized later
  m_ExpNo(0), // modified later
  m_MuonPlusPar(NULL), // modified later
  m_MuonMinusPar(NULL), // modified later
  m_PionPlusPar(NULL), // modified later
  m_PionMinusPar(NULL), // modified later
  m_KaonPlusPar(NULL), // modified later
  m_KaonMinusPar(NULL), // modified later
  m_ProtonPar(NULL), // modified later
  m_AntiprotonPar(NULL), // modified later
  m_DeuteronPar(NULL), // modified later
  m_AntideuteronPar(NULL), // modified later
  m_ElectronPar(NULL), // modified later
  m_PositronPar(NULL) // modified later
{
  for (int j = 0; j < NLAYER + 1; ++j) {
    m_BarrelPhiStripVariance[j] = 0.0;
    m_BarrelZStripVariance[j] = 0.0;
    m_BarrelPhiStripVariance[j] = 0.0;
    m_EndcapModuleMiddleZ[j] = 0.0;
  }
  for (int s = 0; s < NSECTOR + 1; ++s) {
    for (int j = 0; j < NLAYER + 1; ++j) {
      m_BarrelModuleMiddleRadius[0][s][j] = 0.0;
      m_BarrelModuleMiddleRadius[1][s][j] = 0.0;
    }
    m_BarrelSectorPerp[s] = G4ThreeVector(0.0, 0.0, 0.0);
    m_BarrelSectorPhi[s] = G4ThreeVector(0.0, 0.0, 0.0);
  }
}

TrackExtrapolateG4e::~TrackExtrapolateG4e()
{
}

// Initialize for EXT
void TrackExtrapolateG4e::initialize(double minPt, double minKE,
                                     std::vector<Const::ChargedStable>& hypotheses)
{

  m_ExtInitialized = true;

  // Register output and relation arrays
  StoreArray<ExtHit> extHits(*m_ExtHitsColName);
  StoreArray<Track> tracks(*m_TracksColName);
  StoreArray<RecoTrack> recoTracks(*m_RecoTracksColName);
  extHits.registerInDataStore();
  tracks.registerRelationTo(extHits);
  RecoTrack::registerRequiredRelations(recoTracks);

  // Save the magnetic field z component (gauss) at the origin
  m_MagneticField = BFieldManager::getField(B2Vector3D(0, 0, 0)).Z() / Unit::T * CLHEP::tesla / CLHEP::gauss;

  // Convert user cutoff values to geant4 units
  m_MinPt = max(0.0, minPt) * CLHEP::GeV;
  m_MinKE = max(0.0, minKE) * CLHEP::GeV;

  // Save pointer to the list of particle hypotheses for EXT extrapolation
  m_HypothesesExt = &hypotheses;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Store the address of the ExtManager (used later)
  m_ExtMgr = Simulation::ExtManager::GetManager();

  // Set up the EXT-specific geometry (might have already been done by MUID)
  if (m_TargetExt == NULL) {
    GearDir coilContent = GearDir("/Detector/DetectorComponent[@name=\"COIL\"]/Content/");
    double offsetZ = coilContent.getLength("OffsetZ") * CLHEP::cm;
    double rMaxCoil = coilContent.getLength("Cryostat/Rmin") * CLHEP::cm;
    double halfLength = coilContent.getLength("Cryostat/HalfLength") * CLHEP::cm;
    m_TargetExt = new Simulation::ExtCylSurfaceTarget(rMaxCoil, offsetZ - halfLength, offsetZ + halfLength);
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetExt);
    GearDir beampipeContent = GearDir("/Detector/DetectorComponent[@name=\"BeamPipe\"]/Content/");
    double beampipeRadius = beampipeContent.getLength("Lv2OutBe/R2", 1.20) * CLHEP::cm; // mm
    m_MinRadiusSq = beampipeRadius * beampipeRadius; // mm^2
  }

}

// Initialize for MUID
void TrackExtrapolateG4e::initialize(double meanDt, double maxDt, double maxKLMTrackHitDistance,
                                     double maxKLMTrackClusterDistance, double maxECLTrackClusterDistance,
                                     double minPt, double minKE,
                                     std::vector<Const::ChargedStable>& hypotheses)
{

  m_MuidInitialized = true;

  // Register output and relation arrays' persistence
  StoreArray<Track> tracks(*m_TracksColName);
  StoreArray<RecoTrack> recoTracks(*m_RecoTracksColName);
  StoreArray<ExtHit> extHits(*m_ExtHitsColName);
  StoreArray<Muid> muids(*m_MuidsColName);
  StoreArray<MuidHit> muidHits(*m_MuidHitsColName);
  StoreArray<BKLMHit2d> bklmHits(*m_BKLMHitsColName);
  StoreArray<EKLMHit2d> eklmHits(*m_EKLMHitsColName);
  StoreArray<KLMCluster> klmClusters(*m_KLMClustersColName);
  StoreArray<ECLCluster> eclClusters(*m_ECLClustersColName);
  StoreArray<TrackClusterSeparation> trackClusterSeparations(*m_TrackClusterSeparationsColName);
  extHits.registerInDataStore();
  muids.registerInDataStore();
  muidHits.registerInDataStore();
  bklmHits.registerInDataStore();
  eklmHits.registerInDataStore();
  klmClusters.registerInDataStore();
  eclClusters.registerInDataStore();
  trackClusterSeparations.registerInDataStore();
  tracks.registerRelationTo(extHits);
  tracks.registerRelationTo(muids);
  tracks.registerRelationTo(muidHits);
  tracks.registerRelationTo(bklmHits);
  tracks.registerRelationTo(eklmHits);
  tracks.registerRelationTo(trackClusterSeparations);
  tracks.registerRelationTo(klmClusters);
  klmClusters.registerRelationTo(trackClusterSeparations);
  eclClusters.registerRelationTo(extHits);
  RecoTrack::registerRequiredRelations(recoTracks);

  // Save the in-time cut's central value and width for valid hits
  m_MeanDt = meanDt;
  m_MaxDt = maxDt;

  // Save the magnetic field z component (gauss) at the origin
  m_MagneticField = BFieldManager::getField(B2Vector3D(0, 0, 0)).Z() / Unit::T * CLHEP::tesla / CLHEP::gauss;

  // Convert from sigma to variance for hit-position uncertainty
  m_MaxDistSqInVariances = maxKLMTrackHitDistance * maxKLMTrackHitDistance;

  // Convert user cutoff values to geant4 units
  m_MaxKLMTrackClusterDistance = max(0.0, maxKLMTrackClusterDistance) * CLHEP::cm;
  m_MaxECLTrackClusterDistance = max(0.0, maxECLTrackClusterDistance) * CLHEP::cm;
  m_MinPt = max(0.0, minPt) * CLHEP::GeV;
  m_MinKE = max(0.0, minKE) * CLHEP::GeV;

  // Save pointer to the list of particle hypotheses for EXT extrapolation
  m_HypothesesMuid = &hypotheses;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Store the address of the ExtManager (used later)
  m_ExtMgr = Simulation::ExtManager::GetManager();

  // Set up the EXT-specific geometry (might have already been done by EXT)
  if (m_TargetExt == NULL) {
    GearDir coilContent = GearDir("/Detector/DetectorComponent[@name=\"COIL\"]/Content/");
    double offsetZ = coilContent.getLength("OffsetZ") * CLHEP::cm;
    double rMaxCoil = coilContent.getLength("Cryostat/Rmin") * CLHEP::cm;
    double halfLength = coilContent.getLength("Cryostat/HalfLength") * CLHEP::cm;
    m_TargetExt = new Simulation::ExtCylSurfaceTarget(rMaxCoil, offsetZ - halfLength, offsetZ + halfLength);
    GearDir beampipeContent = GearDir("/Detector/DetectorComponent[@name=\"BeamPipe\"]/Content/");
    double beampipeRadius = beampipeContent.getLength("Lv2OutBe/R2", 1.20) * CLHEP::cm; // mm
    m_MinRadiusSq = beampipeRadius * beampipeRadius; // mm^2
  }

  // Set up the MUID-specific geometry
  bklm::GeometryPar* bklmGeometry = bklm::GeometryPar::instance();
  const EKLM::GeometryData& eklmGeometry = EKLM::GeometryData::Instance();
  m_BarrelHalfLength = bklmGeometry->getHalfLength() * CLHEP::cm; // in G4 units (mm)
  m_EndcapHalfLength = 0.5 * eklmGeometry.getEndcapPosition()->getLength(); // in G4 units (mm)
  m_OffsetZ = bklmGeometry->getOffsetZ() * CLHEP::cm; // in G4 units (mm)
  double minZ = m_OffsetZ - (m_BarrelHalfLength + 2.0 * m_EndcapHalfLength);
  double maxZ = m_OffsetZ + (m_BarrelHalfLength + 2.0 * m_EndcapHalfLength);
  m_BarrelNSector = bklmGeometry->getNSector();
  m_BarrelMaxR = bklmGeometry->getOuterRadius() * CLHEP::cm / std::cos(M_PI / m_BarrelNSector); // in G4 units (mm)
  m_TargetMuid = new Simulation::ExtCylSurfaceTarget(m_BarrelMaxR, minZ, maxZ);
  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetMuid);

  m_BarrelHalfLength /= CLHEP::cm;                   // now in G4e units (cm)
  m_EndcapHalfLength /= CLHEP::cm;                   // now in G4e units (cm)
  m_OffsetZ /= CLHEP::cm;                            // now in G4e units (cm)
  m_BarrelMinR = bklmGeometry->getGap1InnerRadius(); // in G4e units (cm)
  m_BarrelMaxR /= CLHEP::cm;                         // now in G4e units (cm)
  m_EndcapMinR = eklmGeometry.getEndcapPosition()->getInnerR() / CLHEP::cm; // in G4e units (cm)
  m_EndcapMaxR = eklmGeometry.getEndcapPosition()->getOuterR() / CLHEP::cm; // in G4e units (cm)
  m_EndcapMiddleZ = m_BarrelHalfLength + m_EndcapHalfLength;                // in G4e units (cm)

  // Measurement uncertainties and acceptance windows
  double width = eklmGeometry.getStripGeometry()->getWidth() / CLHEP::cm; // in G4e units (cm)
  m_EndcapScintVariance = width * width / 12.0;
  width = bklmGeometry->getScintHalfWidth() * 2.0;                        // in G4e units (cm)
  m_BarrelScintVariance = width * width / 12.0;
  int nBarrelLayers = bklmGeometry->getNLayer();
  for (int layer = 1; layer <= nBarrelLayers; ++layer) {
    const bklm::Module* module = bklmGeometry->findModule(layer, false);
    width = module->getPhiStripWidth(); // in G4e units (cm)
    m_BarrelPhiStripVariance[layer - 1] = width * width / 12.0;
    width = module->getZStripWidth(); // in G4e units (cm)
    m_BarrelZStripVariance[layer - 1] = width * width / 12.0;
  }

  // KLM geometry (for associating KLM hit with extrapolated crossing point)

  m_OutermostActiveBarrelLayer = nBarrelLayers - 1; // zero-based counting
  for (int sector = 1; sector <= m_BarrelNSector; ++sector) {
    for (int layer = 1; layer <= nBarrelLayers; ++layer) {
      m_BarrelModuleMiddleRadius[BKLM_FORWARD - 1][sector - 1][layer - 1] = bklmGeometry->getActiveMiddleRadius(BKLM_FORWARD, sector,
          layer); // in G4e units (cm)
      m_BarrelModuleMiddleRadius[BKLM_BACKWARD - 1][sector - 1][layer - 1] = bklmGeometry->getActiveMiddleRadius(BKLM_BACKWARD, sector,
          layer); // in G4e units (cm)
    }
  }
  double dz(eklmGeometry.getLayerShiftZ() / CLHEP::cm); // in G4e units (cm)
  double z0((eklmGeometry.getEndcapPosition()->getZ()
             + eklmGeometry.getLayerShiftZ()
             - 0.5 * eklmGeometry.getEndcapPosition()->getLength()
             - 0.5 * eklmGeometry.getLayerPosition()->getLength()
             - 0.5 * eklmGeometry.getStripGeometry()->getThickness()
             - 0.5 * eklmGeometry.getPlasticSheetGeometry()->getWidth()) / CLHEP::cm); // in G4e units (cm)

  int nEndcapLayers = eklmGeometry.getNLayers();
  m_OutermostActiveForwardEndcapLayer = eklmGeometry.getNDetectorLayers(2) - 1; // zero-based counting
  m_OutermostActiveBackwardEndcapLayer = eklmGeometry.getNDetectorLayers(1) - 1; // zero-based counting

  for (int layer = 1; layer <= nEndcapLayers; ++layer) {
    m_EndcapModuleMiddleZ[layer - 1] = z0 + dz * (layer - 1); // in G4e units (cm)
  }
  for (int sector = 1; sector <= 8; ++sector) {
    double phi = M_PI_4 * (sector - 1);
    m_BarrelSectorPerp[sector - 1].set(std::cos(phi), std::sin(phi), 0.0);
    m_BarrelSectorPhi[sector - 1].set(-std::sin(phi), std::cos(phi), 0.0);
  }
}

void TrackExtrapolateG4e::beginRun(bool byMuid)
{
  StoreObjPtr<EventMetaData> evtMetaData;
  int expNo = evtMetaData->getExperiment();
  B2DEBUG(1, (byMuid ? "muid" : "ext") << ": Experiment " << expNo << "  run " << evtMetaData->getRun());
  if (byMuid) {
    if (m_MuonPlusPar != NULL) {
      if (m_ExpNo == expNo) { return; }
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
    m_ExpNo = expNo;
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
}

void TrackExtrapolateG4e::event(bool byMuid)
{

  // Put geant4 in proper state (in case this module is in a separate process)
  if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  G4ThreeVector directionAtIP, positionG4e, momentumG4e;
  G4ErrorTrajErr covG4e(5); // initialized to zeroes

  StoreArray<Track> tracks(*m_TracksColName);

  // Loop over the reconstructed tracks
  // Do extrapolation for each hypothesis of each reconstructed track.

  if (byMuid) { // event() called by Muid module
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetMuid);
    StoreArray<ECLCluster> eclClusters(*m_ECLClustersColName);
    std::vector<std::pair<ECLCluster*, G4ThreeVector> > eclClusterInfo(eclClusters.getEntries());
    for (int c = 0; c < eclClusters.getEntries(); ++c) {
      eclClusterInfo[c].first = eclClusters[c];
      eclClusterInfo[c].second = G4ThreeVector(eclClusters[c]->getClusterPosition().X(),
                                               eclClusters[c]->getClusterPosition().Y(),
                                               eclClusters[c]->getClusterPosition().Z()) * CLHEP::cm;
    }
    StoreArray<KLMCluster> klmClusters(*m_KLMClustersColName);
    std::vector<std::pair<KLMCluster*, G4ThreeVector> > klmClusterInfo(klmClusters.getEntries());
    for (int c = 0; c < klmClusters.getEntries(); ++c) {
      klmClusterInfo[c].first = klmClusters[c];
      klmClusterInfo[c].second = G4ThreeVector(klmClusters[c]->getClusterPosition().X(),
                                               klmClusters[c]->getClusterPosition().Y(),
                                               klmClusters[c]->getClusterPosition().Z()) * CLHEP::cm;
    }
    // Keep track of (re-)use of BKLMHit2ds
    StoreArray<BKLMHit2d> bklmHits(*m_BKLMHitsColName);
    std::vector<std::map<const Track*, double> > bklmHitUsed(bklmHits.getEntries());
    for (auto& b2track : tracks) {
      for (const auto& hypothesis : *m_HypothesesMuid) {
        int pdgCode = hypothesis.getPDGCode();
        if (hypothesis == Const::electron || hypothesis == Const::muon) pdgCode = -pdgCode;
        G4ErrorFreeTrajState g4eState("g4e_mu+", G4ThreeVector(), G4ThreeVector()); // will be updated
        ExtState extState = getStartPoint(b2track, pdgCode, g4eState);
        swim(extState, g4eState, &eclClusterInfo, &klmClusterInfo, &bklmHitUsed);
      } // Muid hypothesis loop
    } // Muid track loop
  } else { // event() called by Ext module
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetExt);
    for (auto& b2track : tracks) {
      for (const auto& hypothesis : *m_HypothesesExt) {
        int pdgCode = hypothesis.getPDGCode();
        if (hypothesis == Const::electron || hypothesis == Const::muon) pdgCode = -pdgCode;
        G4ErrorFreeTrajState g4eState("g4e_mu+", G4ThreeVector(), G4ThreeVector()); // will be updated
        ExtState extState = getStartPoint(b2track, pdgCode, g4eState);
        swim(extState, g4eState);
      } // Ext hypothesis loop
    } // Ext track loop
  } // byMuid

}

void TrackExtrapolateG4e::endRun(bool)
{
}

void TrackExtrapolateG4e::terminate(bool byMuid)
{

  if (m_DefaultName != NULL) { delete m_DefaultName; }
  if (m_DefaultHypotheses != NULL) { delete m_DefaultHypotheses; }

  if (byMuid) {
    delete m_TargetMuid;
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
  if (m_TargetExt != NULL) {
    delete m_TargetExt;
    m_TargetExt = NULL;
  }
  if (m_EnterExit != NULL) {
    delete m_EnterExit;
    delete m_BKLMVolumes;
    delete m_EKLMVolumes;
    m_ExtMgr->RunTermination();
    m_EnterExit = NULL;
    m_BKLMVolumes = NULL;
    m_EKLMVolumes = NULL;
  }

}

void TrackExtrapolateG4e::extrapolate(int pdgCode, // signed for charge
                                      double tof, // in ns (from IP to position)
                                      // DIVOT bool isCosmic, // true for back-extrapolation
                                      const G4ThreeVector& position, // in cm (genfit2 units)
                                      const G4ThreeVector& momentum, // in GeV/c (genfit2 units)
                                      const G4ErrorSymMatrix& covariance, // (6x6) using cm, GeV/c (genfit2 units)
                                      const std::string&) // DIVOT: NO LONGER USED - REMOVE THIS ARGUMENT
{

  bool isCosmic = false; // DIVOT
  if ((!m_ExtInitialized) && (!m_MuidInitialized)) {
    // No EXT nor MUID module in analysis path ==> mimic ext::initialize() with reasonable defaults.
    // The default values are taken from the EXT module's parameter definitions.
    Simulation::ExtManager* extMgr = Simulation::ExtManager::GetManager();
    extMgr->Initialize("Ext", "default", 0.0, 0.25, false, 0, vector<string>());
    // Redefine geant4e step length, magnetic field step limitation (fraction of local curvature radius),
    // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
    // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
    // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/stepLength 250 mm");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");
    m_DefaultName = new std::string;
    setTracksColName(*m_DefaultName);
    setRecoTracksColName(*m_DefaultName);
    setExtHitsColName(*m_DefaultName);
    m_DefaultHypotheses = new std::vector<Const::ChargedStable>; // not used
    initialize(0.1, 0.002, *m_DefaultHypotheses);
  }

  // Put geant4 in proper state (in case this module is in a separate process)
  if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetExt);

  // Do extrapolation for selected hypothesis (pion, electron, muon, kaon, proton,
  // deuteron) for the selected track until calorimeter exit.

  G4ThreeVector positionG4e = position * CLHEP::cm; // convert from genfit2 units (cm) to geant4 units (mm)
  G4ThreeVector momentumG4e = momentum * CLHEP::GeV; // convert from genfit2 units (GeV/c) to geant4 units (MeV/c)
  if (isCosmic) momentumG4e = -momentumG4e;
  G4ErrorSymMatrix covarianceG4e(5, 0); // in Geant4e units (GeV/c, cm)
  fromPhasespaceToG4e(momentum, covariance, covarianceG4e);
  G4String nameG4e("g4e_" + G4ParticleTable::GetParticleTable()->FindParticle(pdgCode)->GetParticleName());
  G4ErrorFreeTrajState g4eState(nameG4e, positionG4e, momentumG4e, covarianceG4e);
  ExtState extState = { NULL, pdgCode, isCosmic, tof, 0.0,                         // for EXT and MUID
                        momentumG4e.unit(), 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false  // for MUID only
                      };
  swim(extState, g4eState);
}

void TrackExtrapolateG4e::identifyMuon(int pdgCode, // signed for charge
                                       double tof, // in ns (from IP to position)
                                       bool isCosmic, // true for back-extrapolation
                                       const G4ThreeVector& position, // in cm (genfit2 units)
                                       const G4ThreeVector& momentum, // in GeV/c (genfit2 units)
                                       const G4ErrorSymMatrix& covariance) // (6x6) using cm, GeV/c (genfit2 units)
{

  if ((!m_ExtInitialized) && (!m_MuidInitialized)) {
    // No EXT nor MUID module in analysis path ==> mimic ext::initialize() with reasonable defaults.
    // The default values are taken from the MUID module's parameter definitions.
    Simulation::ExtManager* extMgr = Simulation::ExtManager::GetManager();
    extMgr->Initialize("Muid", "default", 0.0, 0.25, false, 0, vector<string>());
    // Redefine geant4e step length, magnetic field step limitation (fraction of local curvature radius),
    // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
    // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
    // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/stepLength 250 mm");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");
    m_DefaultName = new std::string;
    setTracksColName(*m_DefaultName);
    setRecoTracksColName(*m_DefaultName);
    setExtHitsColName(*m_DefaultName);
    setMuidsColName(*m_DefaultName);
    setMuidHitsColName(*m_DefaultName);
    setBKLMHitsColName(*m_DefaultName);
    setEKLMHitsColName(*m_DefaultName);
    setKLMClustersColName(*m_DefaultName);
    setECLClustersColName(*m_DefaultName);
    setTrackClusterSeparationsColName(*m_DefaultName);
    m_DefaultHypotheses = new std::vector<Const::ChargedStable>; // not used
    initialize(0.0, 30.0, 3.5, 150.0, 100.0, 0.1, 0.002, *m_DefaultHypotheses);
  }

  // Put geant4 in proper state (in case this module is in a separate process)
  if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
    G4StateManager::GetStateManager()->SetNewState(G4State_GeomClosed);
  }

  G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetMuid);

  // Do extrapolation for selected hypothesis (pion, electron, muon, kaon, proton,
  // deuteron) for the selected track until calorimeter exit.

  G4ThreeVector positionG4e = position * CLHEP::cm; // from genfit2 units (cm) to geant4 units (mm)
  G4ThreeVector momentumG4e = momentum * CLHEP::GeV; // from genfit2 units (GeV/c) to geant4 units (MeV/c)
  if (isCosmic) momentumG4e = -momentumG4e;
  G4ErrorSymMatrix covarianceG4e(5, 0); // in Geant4e units (GeV/c, cm)
  fromPhasespaceToG4e(momentum, covariance, covarianceG4e);
  G4String nameG4e("g4e_" + G4ParticleTable::GetParticleTable()->FindParticle(pdgCode)->GetParticleName());
  G4ErrorFreeTrajState g4eState(nameG4e, positionG4e, momentumG4e, covarianceG4e);
  ExtState extState = { NULL, pdgCode, isCosmic, tof, 0.0,                             // for EXT and MUID
                        momentumG4e.unit(), 0.0, 0, 0, 0, -1, -1, -1, -1, 0, 0, false  // for MUID only
                      };
  swim(extState, g4eState, NULL, NULL, NULL);
}

// Swim one track for MUID until it stops or leaves the KLM-bounding cylinder
void TrackExtrapolateG4e::swim(ExtState& extState, G4ErrorFreeTrajState& g4eState,
                               const std::vector<std::pair<ECLCluster*, G4ThreeVector> >* eclClusterInfo,
                               const std::vector<std::pair<KLMCluster*, G4ThreeVector> >* klmClusterInfo,
                               std::vector<std::map<const Track*, double> >* bklmHitUsed)
{
  if (extState.pdgCode == 0) return;
  if (g4eState.GetMomentum().perp() <= m_MinPt) return;
  if (m_TargetMuid->GetDistanceFromPoint(g4eState.GetPosition()) < 0.0) return;
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(extState.pdgCode);
  double mass = particle->GetPDGMass();
  double minPSq = (mass + m_MinKE) * (mass + m_MinKE) - mass * mass;
  // Create structures for ECLCluster-Track matching
  std::vector<double> eclClusterDistance;
  std::vector<ExtHit> eclHit1, eclHit2, eclHit3;
  if (eclClusterInfo != NULL) {
    eclClusterDistance.resize(eclClusterInfo->size(), 1.0E10); // "positive infinity"
    ExtHit tempExtHit(extState.pdgCode, Const::EDetector::ECL, 0, EXT_FIRST, 0.0,
                      G4ThreeVector(), G4ThreeVector(), G4ErrorSymMatrix(6));
    eclHit1.resize(eclClusterInfo->size(), tempExtHit);
    eclHit2.resize(eclClusterInfo->size(), tempExtHit);
    eclHit3.resize(eclClusterInfo->size(), tempExtHit);
  }
  // Create structures for KLMCluster-Track matching
  std::vector<TrackClusterSeparation> klmHit;
  if (klmClusterInfo != NULL) {
    klmHit.resize(klmClusterInfo->size()); // initialize each to huge distance
  }
  StoreArray<Muid> muids(*m_MuidsColName);
  Muid* muid = muids.appendNew(extState.pdgCode); // rest of this object will be filled later
  if (extState.track != NULL) { extState.track->addRelationTo(muid); }
  G4ErrorMode propagationMode = (extState.isCosmic ? G4ErrorMode_PropBackwards : G4ErrorMode_PropForwards);
  m_ExtMgr->InitTrackPropagation(propagationMode);
  while (true) {
    const G4int        errCode       = m_ExtMgr->PropagateOneStep(&g4eState, propagationMode);
    G4Track*           track         = g4eState.GetG4Track();
    const G4Step*      step          = track->GetStep();
    const G4StepPoint* preStepPoint  = step->GetPreStepPoint();
    const G4StepPoint* postStepPoint = step->GetPostStepPoint();
    G4TouchableHandle  preTouch      = preStepPoint->GetTouchableHandle();
    G4VPhysicalVolume* pVol          = preTouch->GetVolume();
    const G4int        preStatus     = preStepPoint->GetStepStatus();
    const G4int        postStatus    = postStepPoint->GetStepStatus();
    G4ThreeVector      pos           = track->GetPosition(); // this is at postStepPoint
    G4ThreeVector      mom           = track->GetMomentum(); // ditto
    // Ignore the zero-length step by PropagateOneStep() at each boundary
    if (extState.isCosmic) mom = -mom;
    if (step->GetStepLength() > 0.0) {
      double dt = step->GetDeltaTime();
      double dl = step->GetStepLength() / track->GetMaterial()->GetRadlen();
      if (preStatus == fGeomBoundary) {      // first step in this volume?
        if (m_TargetExt->GetDistanceFromPoint(pos) < 0.0) { // only hits outside ECL during muid
          if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
            createExtHit(EXT_ENTER, extState, g4eState, preStepPoint, preTouch);
          }
        }
      }
      if (extState.isCosmic) {
        extState.tof -= dt;
        extState.length -= dl;
      } else {
        extState.tof += dt;
        extState.length += dl;
      }
      if (postStatus == fGeomBoundary) { // last step in this volume?
        if (m_TargetExt->GetDistanceFromPoint(pos) < 0.0) { // only hits outside ECL during muid
          if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
            createExtHit(EXT_EXIT, extState, g4eState, postStepPoint, preTouch);
          }
        }
      }
      if (createMuidHit(extState, g4eState, bklmHitUsed)) {
        // Force geant4e to update its G4Track from the Kalman-updated state
        m_ExtMgr->GetPropagator()->SetStepN(0);
      }
      if (eclClusterInfo != NULL) {
        for (unsigned int c = 0; c < eclClusterInfo->size(); ++c) {
          G4ThreeVector eclPos((*eclClusterInfo)[c].second);
          G4ThreeVector prePos(preStepPoint->GetPosition());
          G4ThreeVector diff(prePos - eclPos);
          double distance = diff.mag();
          if (distance < m_MaxECLTrackClusterDistance) {
            // fallback ECLNEAR in case no ECLCROSS is found
            if (distance < eclClusterDistance[c]) {
              eclClusterDistance[c] = distance;
              G4ErrorSymMatrix covariance(6, 0);
              fromG4eToPhasespace(g4eState, covariance);
              eclHit3[c].update(EXT_ECLNEAR, extState.tof, pos / CLHEP::cm, mom / CLHEP::GeV, covariance);
            }
            // find position of crossing of the track with the ECLCluster's sphere
            if (eclHit1[c].getStatus() == EXT_FIRST) {
              if (pos.mag2() >= eclPos.mag2()) {
                double r = eclPos.mag();
                double preD = prePos.mag() - r;
                double postD = pos.mag() - r;
                double f = postD / (postD - preD);
                G4ThreeVector midPos = pos + (prePos - pos) * f;
                double tof = extState.tof + dt * f * (extState.isCosmic ? +1 : -1); // in ns, at end of step
                G4ErrorSymMatrix covariance(6, 0);
                fromG4eToPhasespace(g4eState, covariance);
                eclHit1[c].update(EXT_ECLCROSS, tof, midPos / CLHEP::cm, mom / CLHEP::GeV, covariance);
              }
            }
          }
          // find closest distance to the radial line to the ECLCluster
          if (eclHit2[c].getStatus() == EXT_FIRST) {
            G4ThreeVector delta(pos - prePos);
            G4ThreeVector perp(eclPos.cross(delta));
            double perpMag2 = perp.mag2();
            if (perpMag2 > 1.0E-10) {
              double dist = std::fabs(diff * perp) / std::sqrt(perpMag2);
              if (dist < m_MaxECLTrackClusterDistance) {
                double f = eclPos * (prePos.cross(perp)) / perpMag2;
                if ((f > -0.5) && (f <= 1.0)) {
                  G4ThreeVector midPos(prePos + f * delta);
                  double length = extState.length + dl * (1.0 - f) * (extState.isCosmic ? +1 : -1);
                  G4ErrorSymMatrix covariance(6, 0);
                  fromG4eToPhasespace(g4eState, covariance);
                  eclHit2[c].update(EXT_ECLDL, length, midPos / CLHEP::cm, mom / CLHEP::GeV, covariance);
                }
              }
            }
          }
        }
      }
      if (klmClusterInfo != NULL) {
        for (unsigned int c = 0; c < klmClusterInfo->size(); ++c) {
          G4ThreeVector klmPos = (*klmClusterInfo)[c].second;
          G4ThreeVector separation = klmPos - pos;
          double distance = separation.mag();
          if (distance < klmHit[c].getDistance()) {
            klmHit[c].setDistance(distance);
            klmHit[c].setTrackClusterAngle(mom.angle(separation));
            klmHit[c].setTrackClusterSeparationAngle(mom.angle(klmPos));
            klmHit[c].setTrackRotationAngle(extState.directionAtIP.angle(mom));
            klmHit[c].setTrackClusterInitialSeparationAngle(extState.directionAtIP.angle(klmPos));
          }
        }
      }
    }
    // Post-step momentum too low?
    if (errCode || (mom.mag2() < minPSq)) {
      break;
    }
    // Detect escapes from the imaginary target cylinder.
    if (m_TargetMuid->GetDistanceFromPoint(pos) < 0.0) {
      break;
    }
    // Stop extrapolating as soon as the track curls inward too much
    if (pos.perp2() < m_MinRadiusSq) {
      break;
    }
  } // track-extrapolation "infinite" loop

  m_ExtMgr->EventTermination(propagationMode);

  finishTrack(extState, muid, (g4eState.GetPosition().z() > m_OffsetZ));

  if (eclClusterInfo != NULL) {
    StoreArray<ExtHit> extHits(*m_ExtHitsColName);
    for (unsigned int c = 0; c < eclClusterInfo->size(); ++c) {
      if (eclHit1[c].getStatus() != EXT_FIRST) {
        ExtHit* h = extHits.appendNew(eclHit1[c]);
        (*eclClusterInfo)[c].first->addRelationTo(h);
        extState.track->addRelationTo(h);
      }
      if (eclHit2[c].getStatus() != EXT_FIRST) {
        ExtHit* h = extHits.appendNew(eclHit2[c]);
        (*eclClusterInfo)[c].first->addRelationTo(h);
        extState.track->addRelationTo(h);
      }
      if (eclHit3[c].getStatus() != EXT_FIRST) {
        ExtHit* h = extHits.appendNew(eclHit3[c]);
        (*eclClusterInfo)[c].first->addRelationTo(h);
        extState.track->addRelationTo(h);
      }
    }
  }

  if (klmClusterInfo != NULL) {
    StoreArray<TrackClusterSeparation> trackClusterSeparations(*m_TrackClusterSeparationsColName);
    for (unsigned int c = 0; c < klmClusterInfo->size(); ++c) {
      if (klmHit[c].getDistance() > 1.0E9) continue;
      TrackClusterSeparation* h = trackClusterSeparations.appendNew(klmHit[c]);
      (*klmClusterInfo)[c].first->addRelationTo(h); // relation KLMCluster to TrackSep
      extState.track->addRelationTo(h); // relation track to TrackSep
      if (klmHit[c].getDistance() < m_MaxKLMTrackClusterDistance) {
        // relation track->KLMCluster, the MDST obj KLMCluster assumes these exist for at least muons
        extState.track->addRelationTo((*klmClusterInfo)[c].first);
      }
    }
  }

}

// Swim one track for EXT until it stops or leaves the ECL-bounding  cylinder
void TrackExtrapolateG4e::swim(ExtState& extState, G4ErrorFreeTrajState& g4eState)
{
  if (extState.pdgCode == 0) return;
  if (g4eState.GetMomentum().perp() <= m_MinPt) return;
  if (m_TargetExt->GetDistanceFromPoint(g4eState.GetPosition()) < 0.0) return;
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(extState.pdgCode);
  double mass = particle->GetPDGMass();
  double minPSq = (mass + m_MinKE) * (mass + m_MinKE) - mass * mass;
  G4ErrorMode propagationMode = (extState.isCosmic ? G4ErrorMode_PropBackwards : G4ErrorMode_PropForwards);
  m_ExtMgr->InitTrackPropagation(propagationMode);
  while (true) {
    const G4int        errCode       = m_ExtMgr->PropagateOneStep(&g4eState, propagationMode);
    G4Track*           track         = g4eState.GetG4Track();
    const G4Step*      step          = track->GetStep();
    const G4StepPoint* preStepPoint  = step->GetPreStepPoint();
    const G4StepPoint* postStepPoint = step->GetPostStepPoint();
    G4TouchableHandle  preTouch      = preStepPoint->GetTouchableHandle();
    G4VPhysicalVolume* pVol          = preTouch->GetVolume();
    const G4int        preStatus     = preStepPoint->GetStepStatus();
    const G4int        postStatus    = postStepPoint->GetStepStatus();
    G4ThreeVector      pos           = track->GetPosition(); // this is at postStepPoint
    G4ThreeVector      mom           = track->GetMomentum(); // ditto
    // First step on this track?
    if (extState.isCosmic) mom = -mom;
    if (preStatus == fUndefined) {
      if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
        createExtHit(EXT_FIRST, extState, g4eState, preStepPoint, preTouch);
      }
    }
    // Ignore the zero-length step by PropagateOneStep() at each boundary
    if (step->GetStepLength() > 0.0) {
      double dt = step->GetDeltaTime();
      double dl = step->GetStepLength() / track->GetMaterial()->GetRadlen();
      if (preStatus == fGeomBoundary) {      // first step in this volume?
        if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
          createExtHit(EXT_ENTER, extState, g4eState, preStepPoint, preTouch);
        }
      }
      if (extState.isCosmic) {
        extState.tof -= dt;
        extState.length -= dl;
      } else {
        extState.tof += dt;
        extState.length += dl;
      }
      // Last step in this volume?
      if (postStatus == fGeomBoundary) {
        if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
          createExtHit(EXT_EXIT, extState, g4eState, postStepPoint, preTouch);
        }
      }
    }
    // Post-step momentum too low?
    if (errCode || (mom.mag2() < minPSq)) {
      if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
        createExtHit(EXT_STOP, extState, g4eState, postStepPoint, preTouch);
      }
      break;
    }
    // Detect escapes from the imaginary target cylinder.
    if (m_TargetExt->GetDistanceFromPoint(pos) < 0.0) {
      if (m_EnterExit->find(pVol) != m_EnterExit->end()) {
        createExtHit(EXT_ESCAPE, extState, g4eState, postStepPoint, preTouch);
      }
      break;
    }
    // Stop extrapolating as soon as the track curls inward too much
    if (pos.perp2() < m_MinRadiusSq) {
      break;
    }
  } // track-extrapolation "infinite" loop

  m_ExtMgr->EventTermination(propagationMode);

}

// Register the list of volumes for which entry/exit point is to be saved during extrapolation
void TrackExtrapolateG4e::registerVolumes()
{

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  if (pvStore->size() == 0) {
    B2FATAL("No geometry defined. Please create the geometry first.");
  }

  if (m_EnterExit != NULL) { return; } // Only do this once

  m_EnterExit = new map<G4VPhysicalVolume*, enum VolTypes>;
  m_BKLMVolumes = new vector<G4VPhysicalVolume*>;
  m_EKLMVolumes = new vector<G4VPhysicalVolume*>;
  for (vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();

    // CDC volumes have "CDC" in the name
    if (name.find("CDC") != string::npos) {
      // DO NOT STORE CDC HITS
      //  (*m_EnterExit)[*iVol] = VOLTYPE_CDC;
    }
    // TOP doesn't have one envelope; it has 16 "TOPModule"s
    else if (name.find("TOPModule") != string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_TOP1;
    }
    // TOP quartz bar (=sensitive)
    else if (name.find("_TOPPrism_") != string::npos ||
             name.find("_TOPBarSegment") != string::npos ||
             name.find("_TOPMirrorSegment") != string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_TOP2;
    }
    // TOP quartz glue (not sensitive?)
    else if (name.find("TOPBarSegment1Glue") != string::npos ||
             name.find("TOPBarSegment2Glue") != string::npos ||
             name.find("TOPMirrorSegmentGlue") != string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_TOP3;
      // ARICH volumes
    } else if (name == "ARICH.AerogelSupportPlate") {
      (*m_EnterExit)[*iVol] = VOLTYPE_ARICH1;
    } else if (name == "ARICH.AerogelImgPlate") {
      (*m_EnterExit)[*iVol] = VOLTYPE_ARICH2;
    } else if (name.find("ARICH.HAPDWindow") != string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_ARICH3;
    }

    // ECL crystal
    else if (name.find("lv_barrel_crystal_") != string::npos ||
             name.find("lv_forward_crystal_") != string::npos ||
             name.find("lv_backward_crystal_") != string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_ECL;
    }
    // Barrel KLM: BKLM.Layer**GasPhysical for RPCs or BKLM.Layer**ChimneyGasPhysical for RPCs
    //             BKLM.ScintActiveType*Physical for scintillator strips
    else if (name.compare(0, 5, "BKLM.") == 0) {
      if (name.find("GasPhysical") != string::npos) {
        (*m_EnterExit)[*iVol] = VOLTYPE_BKLM1;
      } else if (name.find("ScintActiveType") != string::npos) {
        (*m_EnterExit)[*iVol] = VOLTYPE_BKLM2;
      } else if ((name.find("ScintType") != string::npos) ||
                 (name.find("ElectrodePhysical") != string::npos)) {
        m_BKLMVolumes->push_back(*iVol);
      }
    }
    // Endcap KLM: StripSensitive_*
    else if (name.compare(0, 14, "StripSensitive") == 0) {
      (*m_EnterExit)[*iVol] = VOLTYPE_EKLM;
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

  G4VPhysicalVolume* pv = touch->GetVolume(0);
  std::map<G4VPhysicalVolume*, enum VolTypes>::iterator it = m_EnterExit->find(pv);
  if (it == m_EnterExit->end()) { return; }

  switch (it->second) {
    case VOLTYPE_CDC:
      detID = Const::EDetector::CDC;
      copyID = pv->GetCopyNo();
      return;
    case VOLTYPE_TOP1:
      detID = Const::EDetector::TOP;
      copyID = -(pv->GetCopyNo()); // negative to distinguish module and quartz hits
      return;
    case VOLTYPE_TOP2:
      detID = Const::EDetector::TOP;
      if (touch->GetHistoryDepth() >= 1) copyID = touch->GetVolume(1)->GetCopyNo();
      return;
    case VOLTYPE_TOP3:
      detID = Const::EDetector::TOP;
      if (touch->GetHistoryDepth() >= 2) copyID = touch->GetVolume(2)->GetCopyNo();
      return;
    case VOLTYPE_ARICH1:
      detID = Const::EDetector::ARICH;
      copyID = 12345;
      return;
    case VOLTYPE_ARICH2:
      detID = Const::EDetector::ARICH;
      copyID = 6789;
      return;
    case VOLTYPE_ARICH3:
      detID = Const::EDetector::ARICH;
      if (touch->GetHistoryDepth() >= 2) copyID = touch->GetVolume(2)->GetCopyNo();
      return;
    case VOLTYPE_ECL:
      detID = Const::EDetector::ECL;
      copyID = ECL::ECLGeometryPar::Instance()->ECLVolumeToCellID(touch());
      return;
    case VOLTYPE_BKLM1: // BKLM RPCs
      detID = Const::EDetector::BKLM;
      if (touch->GetHistoryDepth() == DEPTH_RPC) {
        // int plane = touch->GetCopyNumber(0);
        int layer = touch->GetCopyNumber(4);
        int sector = touch->GetCopyNumber(6);
        bool isForward = (touch->GetCopyNumber(7) == BKLM_FORWARD);
        copyID = (isForward ? BKLM_END_MASK : 0)
                 | ((sector - 1) << BKLM_SECTOR_BIT)
                 | ((layer - 1) << BKLM_LAYER_BIT)
                 | BKLM_INRPC_MASK
                 | BKLM_MC_MASK;
      }
      return;
    case VOLTYPE_BKLM2: // BKLM scints
      detID = Const::EDetector::BKLM;
      if (touch->GetHistoryDepth() == DEPTH_SCINT) {
        int scint = touch->GetCopyNumber(1);
        int plane = touch->GetCopyNumber(2);
        int layer = touch->GetCopyNumber(6);
        int sector = touch->GetCopyNumber(8);
        bool isForward = (touch->GetCopyNumber(9) == BKLM_FORWARD);
        copyID = (isForward ? BKLM_END_MASK : 0)
                 | ((sector - 1) << BKLM_SECTOR_BIT)
                 | ((layer - 1) << BKLM_LAYER_BIT)
                 | ((scint - 1) << BKLM_STRIP_BIT)
                 | ((scint - 1) << BKLM_MAXSTRIP_BIT)
                 | (plane == BKLM_INNER ? BKLM_PLANE_MASK : 0)
                 | BKLM_MC_MASK;
      }
      return;
    case VOLTYPE_EKLM:
      detID = Const::EDetector::EKLM;
      copyID = EKLM::GeometryData::Instance().stripNumber(
                 touch->GetVolume(7)->GetCopyNo(),
                 touch->GetVolume(6)->GetCopyNo(),
                 touch->GetVolume(5)->GetCopyNo(),
                 touch->GetVolume(4)->GetCopyNo(),
                 touch->GetVolume(1)->GetCopyNo());
      return;
  }

}

ExtState TrackExtrapolateG4e::getStartPoint(const Track& b2track, int pdgCode, G4ErrorFreeTrajState& g4eState)
{
  ExtState extState = {&b2track, pdgCode, false, 0.0, 0.0,                               // for EXT and MUID
                       G4ThreeVector(0, 0, 1), 0.0, 0, 0, 0, -1, -1, -1, -1, 0, 0, false // for MUID only
                      };
  RecoTrack* recoTrack = b2track.getRelatedTo<RecoTrack>();
  if (recoTrack == NULL) {
    B2WARNING("Track without associated RecoTrack: skipping extrapolation for this track.");
    extState.pdgCode = 0; // prevent start of extrapolation in swim()
    return extState;
  }
  const genfit::AbsTrackRep* trackRep = recoTrack->getCardinalRepresentation();
  int charge = int(trackRep->getPDGCharge());
  if (charge != 0) {
    extState.pdgCode *= charge;
  } else {
    charge = 1; // should never happen but persist if it does
  }
  TVector3 firstPosition, firstMomentum, lastPosition, lastMomentum; // initialized to zeroes
  TMatrixDSym firstCov(6), lastCov(6);                               // initialized to zeroes
  try {
    const genfit::MeasuredStateOnPlane& firstState = recoTrack->getMeasuredStateOnPlaneFromFirstHit(trackRep);
    trackRep->getPosMomCov(firstState, firstPosition, firstMomentum, firstCov);
    const genfit::MeasuredStateOnPlane& lastState = recoTrack->getMeasuredStateOnPlaneFromLastHit(trackRep);
    trackRep->getPosMomCov(lastState, lastPosition, lastMomentum, lastCov);
    // in genfit units (cm, GeV/c)
    extState.tof = lastState.getTime(); // DIVOT: must be revised when IP profile (reconstructed beam spot) become available!
    if (lastPosition.Mag2() < firstPosition.Mag2()) {
      firstPosition = lastPosition;
      firstMomentum = -lastMomentum;
      firstCov = lastCov;
      trackRep->getPosMomCov(firstState, lastPosition, lastMomentum, lastCov);
      lastMomentum *= -1.0; // extrapolate backwards instead of forwards
      extState.isCosmic = true;
      extState.tof = firstState.getTime(); // DIVOT: must be revised when IP profile (reconstructed beam spot) become available!
    }

    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(extState.pdgCode);
    if (extState.pdgCode != trackRep->getPDG()) {
      double pSq = lastMomentum.Mag2();
      double mass = particle->GetPDGMass() / CLHEP::GeV;
      extState.tof *= std::sqrt((pSq + mass * mass) / (pSq + lastState.getMass() * lastState.getMass()));
    }

    extState.directionAtIP.set(firstMomentum.Unit().X(), firstMomentum.Unit().Y(), firstMomentum.Unit().Z());
    if (m_MagneticField != 0.0) { // in gauss
      double radius = (firstMomentum.Perp() * CLHEP::GeV / CLHEP::eV) / (CLHEP::c_light * charge * m_MagneticField); // in cm
      double centerPhi = extState.directionAtIP.phi() - M_PI_2;
      double centerX = firstPosition.X() + radius * std::cos(centerPhi);
      double centerY = firstPosition.Y() + radius * std::sin(centerPhi);
      double pocaPhi = atan2(charge * centerY, charge * centerX) + M_PI;
      double ipPerp = extState.directionAtIP.perp();
      if (ipPerp > 0.0) {
        extState.directionAtIP.setX(+std::sin(pocaPhi) * ipPerp);
        extState.directionAtIP.setY(-std::cos(pocaPhi) * ipPerp);
      }
    } else {
      // No field: replace flaky covariance matrix with a diagonal one measured in 1.5T field
      // for a 10 GeV/c track ... and replace momentum magnitude with fixed 10 GeV/c
      lastCov *= 0.0;
      lastCov[0][0] = 5.0E-5;
      lastCov[1][1] = 1.0E-7;
      lastCov[2][2] = 5.0E-4;
      lastCov[3][3] = 3.5E-3;
      lastCov[4][4] = 3.5E-3;
      lastMomentum = lastMomentum.Unit() * 10.0;
    }

    G4ThreeVector posG4e(lastPosition.X() * CLHEP::cm, lastPosition.Y() * CLHEP::cm,
                         lastPosition.Z() * CLHEP::cm); // in Geant4 units (mm)
    G4ThreeVector momG4e(lastMomentum.X() * CLHEP::GeV, lastMomentum.Y() * CLHEP::GeV,
                         lastMomentum.Z() * CLHEP::GeV);  // in Geant4 units (MeV/c)
    G4ErrorSymMatrix covG4e(5, 0); // in Geant4e units (GeV/c, cm)
    fromPhasespaceToG4e(lastMomentum, lastCov, covG4e); // in Geant4e units (GeV/c, cm)
    g4eState.SetData("g4e_" + particle->GetParticleName(), posG4e, momG4e);
    g4eState.SetParameters(posG4e, momG4e); // compute private-state parameters from momG4e
    g4eState.SetError(covG4e);
  } catch (genfit::Exception) {
    B2WARNING("genfit::MeasuredStateOnPlane() exception: skipping extrapolation for this track. initial momentum = ("
              << firstMomentum.X() << "," << firstMomentum.Y() << "," << firstMomentum.Z() << ")");
    extState.pdgCode = 0; // prevent start of extrapolation in swim()
  }

  return extState;
}


void TrackExtrapolateG4e::fromG4eToPhasespace(const G4ErrorFreeTrajState& g4eState, G4ErrorSymMatrix& covariance)
{

  // Convert Geant4e covariance matrix with parameters 1/p, lambda, phi, yT, zT (in GeV/c, radians, cm)
  // to phase-space covariance matrix with parameters x, y, z, px, py, pz (in GeV/c, cm)
  // (1/p) = 1/sqrt( px^2 + py^2 + pz^2 )
  // phi = atan( py / px )
  // lambda = asin( pz / sqrt( px^2 + py^2 + pz^2 )
  // xT = x * cos(lambda) * cos(phi) + y * cos(lambda) * sin(phi) + z * sin(lambda)
  // yT = -x * sin(phi) + y * cos(phi)
  // zT = -x * sin(lambda) * cos(phi) - y * sin(lambda) * sin(phi) + z * cos(lambda)

  G4ErrorFreeTrajParam param = g4eState.GetParameters();
  double p = 1.0 / (param.GetInvP() * CLHEP::GeV);     // in GeV/c
  double pSq = p * p;
  double lambda = param.GetLambda();    // in radians
  double sinLambda = std::sin(lambda);
  double cosLambda = std::cos(lambda);
  double phi = param.GetPhi();          // in radians
  double sinPhi = std::sin(phi);
  double cosPhi = std::cos(phi);

  // Transformation Jacobian 6x5 from Geant4e 5x5 to phase-space 6x6

  G4ErrorMatrix jacobian(6, 5, 0); // All entries are initialized to 0

  jacobian(4, 1) = -pSq * cosLambda * cosPhi;          // @(px)/@(1/p)
  jacobian(5, 1) = -pSq * cosLambda * sinPhi;          // @(py)/@(1/p)
  jacobian(6, 1) = -pSq * sinLambda;                   // @(pz)/@(1/p)

  jacobian(4, 2) = -p * sinLambda * cosPhi;            // @(px)/@(lambda)
  jacobian(5, 2) = -p * sinLambda * sinPhi;            // @(py)/@(lambda)
  jacobian(6, 2) =  p * cosLambda;                     // @(pz)/@(lambda)

  jacobian(4, 3) = -p * cosLambda * sinPhi;            // @(px)/@(phi)
  jacobian(5, 3) =  p * cosLambda * cosPhi;            // @(py)/@(phi)

  jacobian(1, 4) = -sinPhi;                            // @(x)/@(yT)
  jacobian(2, 4) =  cosPhi;                            // @(y)/@(yT)

  jacobian(1, 5) = -sinLambda * cosPhi;                // @(x)/@(zT)
  jacobian(2, 5) = -sinLambda * sinPhi;                // @(y)/@(zT)
  jacobian(3, 5) =  cosLambda;                         // @(z)/@(zT)

  G4ErrorTrajErr g4eCov = g4eState.GetError();
  covariance.assign(g4eCov.similarity(jacobian));

}

void TrackExtrapolateG4e::fromPhasespaceToG4e(const TVector3& momentum, const TMatrixDSym& covariance, G4ErrorTrajErr& covG4e)
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
  for (int k = 0; k < 6; ++k) {
    for (int j = k; j < 6; ++j) {
      temp[j][k] = covariance[j][k];
    }
  }

  double pInvSq = 1.0 / momentum.Mag2();
  double pInv   = std::sqrt(pInvSq);
  double pPerpInv = 1.0 / momentum.Perp();
  double sinLambda = momentum.CosTheta();
  double cosLambda = std::sqrt(1.0 - sinLambda * sinLambda);
  double phi = momentum.Phi();
  double cosPhi = std::cos(phi);
  double sinPhi = std::sin(phi);

  // Transformation Jacobian 5x6 from phase-space 6x6 to Geant4e 5x5
  G4ErrorMatrix jacobian(5, 6, 0); // All entries are initialized to 0

  jacobian(1, 4) = -pInvSq * cosLambda * cosPhi;       // @(1/p)/@(px)
  jacobian(1, 5) = -pInvSq * cosLambda * sinPhi;       // @(1/p)/@(py)
  jacobian(1, 6) = -pInvSq * sinLambda;                // @(1/p)/@(pz)

  jacobian(2, 4) = -pInv * sinLambda * cosPhi;         // @(lambda)/@(px)
  jacobian(2, 5) = -pInv * sinLambda * sinPhi;         // @(lambda)/@(py)
  jacobian(2, 6) =  pInv * cosLambda;                  // @(lambda)/@(pz)

  jacobian(3, 4) = -pPerpInv * sinPhi;                 // @(phi)/@(px)
  jacobian(3, 5) =  pPerpInv * cosPhi;                 // @(phi)/@(py)

  jacobian(4, 1) = -sinPhi;                            // @(yT)/@(x)
  jacobian(4, 2) =  cosPhi;                            // @(yT)/@(y)

  jacobian(5, 1) = -sinLambda * cosPhi;                // @(zT)/@(x)
  jacobian(5, 2) = -sinLambda * sinPhi;                // @(zT)/@(y)
  jacobian(5, 3) =  cosLambda;                         // @(zT)/@(z)

  covG4e = temp.similarity(jacobian);

}

void TrackExtrapolateG4e::fromPhasespaceToG4e(const G4ThreeVector& momentum, const G4ErrorSymMatrix& covariance,
                                              G4ErrorTrajErr& covG4e)
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

  double pInvSq = 1.0 / momentum.mag2();
  double pInv   = std::sqrt(pInvSq);
  double pPerpInv = 1.0 / momentum.perp();
  double sinLambda = momentum.cosTheta();
  double cosLambda = std::sqrt(1.0 - sinLambda * sinLambda);
  double phi = momentum.phi();
  double cosPhi = std::cos(phi);
  double sinPhi = std::sin(phi);

  // Transformation Jacobian 5x6 from phase-space 6x6 to Geant4e 5x5
  G4ErrorMatrix jacobian(5, 6, 0);

  jacobian(1, 4) = -pInvSq * cosLambda * cosPhi;       // @(1/p)/@(px)
  jacobian(1, 5) = -pInvSq * cosLambda * sinPhi;       // @(1/p)/@(py)
  jacobian(1, 6) = -pInvSq * sinLambda;                // @(1/p)/@(pz)

  jacobian(2, 4) = -pInv * sinLambda * cosPhi;         // @(lambda)/@(px)
  jacobian(2, 5) = -pInv * sinLambda * sinPhi;         // @(lambda)/@(py)
  jacobian(2, 6) =  pInv * cosLambda;                  // @(lambda)/@(pz)

  jacobian(3, 4) = -pPerpInv * sinPhi;                 // @(phi)/@(px)
  jacobian(3, 5) =  pPerpInv * cosPhi;                 // @(phi)/@(py)

  jacobian(4, 1) = -sinPhi;                            // @(yT)/@(x)
  jacobian(4, 2) =  cosPhi;                            // @(yT)/@(y)

  jacobian(5, 1) = -sinLambda * cosPhi;                // @(zT)/@(x)
  jacobian(5, 2) = -sinLambda * sinPhi;                // @(zT)/@(y)
  jacobian(5, 3) =  cosLambda;                         // @(zT)/@(z)
  covG4e = temp.similarity(jacobian);

}

// write another volume-entry or volume-exit point on extrapolated track
void TrackExtrapolateG4e::createExtHit(ExtHitStatus status, const ExtState& extState,
                                       const G4ErrorFreeTrajState& g4eState,
                                       const G4StepPoint* stepPoint, const G4TouchableHandle& touch)
{

  Const::EDetector detID(Const::EDetector::invalidDetector);
  int copyID(0);
  getVolumeID(touch, detID, copyID);
  G4ThreeVector pos(stepPoint->GetPosition() / CLHEP::cm);
  G4ThreeVector mom(stepPoint->GetMomentum() / CLHEP::GeV);
  if (extState.isCosmic) mom = -mom;
  G4ErrorSymMatrix covariance(6, 0);
  fromG4eToPhasespace(g4eState, covariance);
  StoreArray<ExtHit> extHits(*m_ExtHitsColName);
  ExtHit* extHit = extHits.appendNew(extState.pdgCode, detID, copyID, status,
                                     extState.tof, pos, mom, covariance);
  // If called standalone, there will be no associated track
  if (extState.track != NULL) { extState.track->addRelationTo(extHit); }

}

// Write another volume-entry point on track.
// The track state will be modified here by the Kalman fitter.

bool TrackExtrapolateG4e::createMuidHit(ExtState& extState, G4ErrorFreeTrajState& g4eState,
                                        std::vector<std::map<const Track*, double> >* bklmHitUsed)
{

  Intersection intersection;
  intersection.hit = -1;
  intersection.chi2 = -1.0;
  intersection.position = g4eState.GetPosition() / CLHEP::cm;
  intersection.momentum = g4eState.GetMomentum() / CLHEP::GeV;
  G4ThreeVector prePos = g4eState.GetG4Track()->GetStep()->GetPreStepPoint()->GetPosition() / CLHEP::cm;
  G4ThreeVector oldPosition(prePos.x(), prePos.y(), prePos.z());
  double r = intersection.position.perp();
  double z = std::fabs(intersection.position.z() - m_OffsetZ);

  // Is the track in the barrel?
  if ((r > m_BarrelMinR) && (r < m_BarrelMaxR) && (z < m_BarrelHalfLength)) {
    // Did the track cross the inner midplane of a detector module?
    if (findBarrelIntersection(extState, oldPosition, intersection)) {
      fromG4eToPhasespace(g4eState, intersection.covariance);
      if (findMatchingBarrelHit(intersection, extState.track)) {
        (*bklmHitUsed)[intersection.hit].insert(std::pair<const Track*, double>(extState.track, intersection.chi2));
        extState.extLayerPattern |= (0x00000001 << intersection.layer);
        if (extState.lastBarrelExtLayer < intersection.layer) {
          extState.lastBarrelExtLayer = intersection.layer;
        }
        extState.hitLayerPattern |= (0x00000001 << intersection.layer);
        if (extState.lastBarrelHitLayer < intersection.layer) {
          extState.lastBarrelHitLayer = intersection.layer;
        }
        // If the updated point is outside the barrel, discard it and the Kalman-fitter adjustment
        r = intersection.position.perp();
        z = std::fabs(intersection.position.z() - m_OffsetZ);
        if ((r <= m_BarrelMinR) || (r >= m_BarrelMaxR) || (z >= m_BarrelHalfLength)) {
          intersection.chi2 = -1.0;
        }
      } else {
        // Record a no-hit track crossing if this step is strictly within a barrel sensitive volume
        vector<G4VPhysicalVolume*>::iterator j = find(m_BKLMVolumes->begin(), m_BKLMVolumes->end(), g4eState.GetG4Track()->GetVolume());
        if (j != m_BKLMVolumes->end()) {
          extState.extLayerPattern |= (0x00000001 << intersection.layer);
          if (extState.lastBarrelExtLayer < intersection.layer) {
            extState.lastBarrelExtLayer = intersection.layer;
          }
        }
      }
    }
  }

  // Is the track in the endcap?
  if ((r > m_EndcapMinR) && (std::fabs(z - m_EndcapMiddleZ) < m_EndcapHalfLength)) {
    // Did the track cross the inner midplane of a detector module?
    if (findEndcapIntersection(extState, oldPosition, intersection)) {
      fromG4eToPhasespace(g4eState, intersection.covariance);
      if (findMatchingEndcapHit(intersection, extState.track)) {
        extState.extLayerPattern |= (0x00008000 << intersection.layer);
        if (extState.lastEndcapExtLayer < intersection.layer) {
          extState.lastEndcapExtLayer = intersection.layer;
        }
        extState.hitLayerPattern |= (0x00008000 << intersection.layer);
        if (extState.lastEndcapHitLayer < intersection.layer) {
          extState.lastEndcapHitLayer = intersection.layer;
        }
        // If the updated point is outside the endcap, discard it and the Kalman-fitter adjustment
        r = intersection.position.perp();
        z = std::fabs(intersection.position.z() - m_OffsetZ);
        if ((r <= m_EndcapMinR) || (r >= m_EndcapMaxR) || (std::fabs(z - m_EndcapMiddleZ) >= m_EndcapHalfLength)) {
          intersection.chi2 = -1.0;
        }
      } else {
        // Record a no-hit track crossing if this step is strictly within an endcap sensitive volume
        vector<G4VPhysicalVolume*>::iterator j = find(m_EKLMVolumes->begin(), m_EKLMVolumes->end(), g4eState.GetG4Track()->GetVolume());
        if (j != m_EKLMVolumes->end()) {
          extState.extLayerPattern |= (0x00008000 << intersection.layer);
          if (extState.lastEndcapExtLayer < intersection.layer) {
            extState.lastEndcapExtLayer = intersection.layer;
          }
        }
      }
    }
  }

  // Create a new MuidHit and RelationEntry between it and the track.
  // Adjust geant4e's position, momentum and covariance based on matching hit and tell caller to update the geant4e state.
  if (intersection.chi2 >= 0.0) {
    StoreArray<MuidHit> muidHits(*m_MuidHitsColName);
    TVector3 tpos(intersection.position.x(), intersection.position.y(), intersection.position.z());
    TVector3 tposAtHitPlane(intersection.positionAtHitPlane.x(),
                            intersection.positionAtHitPlane.y(),
                            intersection.positionAtHitPlane.z());
    MuidHit* muidHit = muidHits.appendNew(extState.pdgCode, intersection.inBarrel, intersection.isForward, intersection.sector,
                                          intersection.layer, tpos,
                                          tposAtHitPlane, extState.tof, intersection.time, intersection.chi2);
    if (extState.track != NULL) { extState.track->addRelationTo(muidHit); }
    G4Point3D newPos(intersection.position.x() * CLHEP::cm,
                     intersection.position.y() * CLHEP::cm,
                     intersection.position.z() * CLHEP::cm);
    g4eState.SetPosition(newPos);
    G4Vector3D newMom(intersection.momentum.x() * CLHEP::GeV,
                      intersection.momentum.y() * CLHEP::GeV,
                      intersection.momentum.z() * CLHEP::GeV);
    g4eState.SetMomentum(newMom);
    G4ErrorTrajErr covG4e;
    fromPhasespaceToG4e(intersection.momentum, intersection.covariance, covG4e);
    g4eState.SetError(covG4e);
    extState.chi2 += intersection.chi2;
    extState.nPoint += 2; // two (orthogonal) independent hits per detector layer
    return true;
  }

  // Tell caller that the geant4e state was not modified.
  return false;

}

bool TrackExtrapolateG4e::findBarrelIntersection(ExtState& extState, const G4ThreeVector& oldPosition, Intersection& intersection)
{

  // Be generous: allow outward-moving intersection to be in the dead space between
  // largest sensitive-volume Z and m_BarrelHalfLength, not necessarily in a geant4 sensitive volume

  if (std::fabs(intersection.position.z() - m_OffsetZ) > m_BarrelHalfLength) return false;

  double phi = intersection.position.phi();
  if (phi < 0.0) { phi += TWOPI; }
  if (phi > TWOPI - PI_8) { phi -= TWOPI; }
  int sector = (int)((phi + PI_8) / M_PI_4);
  int fb = (intersection.position.z() > m_OffsetZ ? BKLM_FORWARD : BKLM_BACKWARD) - 1;

  double oldR = oldPosition * m_BarrelSectorPerp[sector];
  double newR = intersection.position * m_BarrelSectorPerp[sector];

  for (int layer = extState.firstBarrelLayer; layer <= m_OutermostActiveBarrelLayer; ++layer) {
    if (newR <  m_BarrelModuleMiddleRadius[fb][sector][layer]) break;
    if (oldR <= m_BarrelModuleMiddleRadius[fb][sector][layer]) {
      extState.firstBarrelLayer = layer + 1; // ratchet outward for next call's loop starting value
      if (extState.firstBarrelLayer > m_OutermostActiveBarrelLayer) extState.escaped = true;
      intersection.inBarrel = true;
      intersection.isForward = intersection.position.z() > m_OffsetZ;
      intersection.layer = layer;
      intersection.sector = sector;
      return true;
    }
  }

  return false;

}

bool TrackExtrapolateG4e::findEndcapIntersection(ExtState& extState, const G4ThreeVector& oldPosition, Intersection& intersection)
{

  // Be generous: allow intersection to be in the dead space between m_EndcapMinR and innermost
  // sensitive-volume radius or between outermost sensitive-volume radius and m_EndcapMaxR,
  // not necessarily in a geant4 sensitive volume

  if (oldPosition.perp() > m_EndcapMaxR) return false;
  if (intersection.position.perp() < m_EndcapMinR) return false;

  double oldZ = std::fabs(oldPosition.z() - m_OffsetZ);
  double newZ = std::fabs(intersection.position.z() - m_OffsetZ);
  bool isForward = intersection.position.z() > m_OffsetZ;
  int outermostLayer = isForward ? m_OutermostActiveForwardEndcapLayer
                       : m_OutermostActiveBackwardEndcapLayer;

  for (int layer = extState.firstEndcapLayer; layer <= outermostLayer; ++layer) {
    if (newZ <  m_EndcapModuleMiddleZ[layer]) break;
    if (oldZ <= m_EndcapModuleMiddleZ[layer]) {
      extState.firstEndcapLayer = layer + 1; // ratchet outward for next call's loop starting value
      if (extState.firstEndcapLayer > outermostLayer) extState.escaped = true;
      intersection.inBarrel = false;
      intersection.isForward = isForward;
      intersection.layer = layer;
      double phi = intersection.position.phi();
      if (phi < 0.0) { phi += TWOPI; }
      if (isForward) {
        phi = M_PI - phi;
        if (phi < 0.0) { phi += TWOPI; }
      }
      intersection.sector = (int)(phi / M_PI_2); // my calculation; matches EKLM-geometry calculation
      return true;
    }
  }

  return false;

}

bool TrackExtrapolateG4e::findMatchingBarrelHit(Intersection& intersection, const Track* track)

{

  G4ThreeVector extPos0(intersection.position);

  StoreArray<BKLMHit2d> bklmHits(*m_BKLMHitsColName);
  double diffBestMagSq = 1.0E60;
  int bestHit = -1;
  int matchingLayer = intersection.layer + 1;
  G4ThreeVector n(m_BarrelSectorPerp[intersection.sector]);
  for (int h = 0; h < bklmHits.getEntries(); ++h) {
    BKLMHit2d* hit = bklmHits[h];
    if (hit->getLayer() != matchingLayer) continue;
    if (hit->isOutOfTime()) continue;
    if (std::fabs(hit->getTime() - m_MeanDt) > m_MaxDt) continue;
    G4ThreeVector diff(hit->getGlobalPositionX() - intersection.position.x(),
                       hit->getGlobalPositionY() - intersection.position.y(),
                       hit->getGlobalPositionZ() - intersection.position.z());
    double dn = diff * n; // in cm
    if (std::fabs(dn) < 2.0) {
      // Hit and extrapolated point are in the same sector
      diff -= n * dn;
      if (diff.mag2() < diffBestMagSq) {
        diffBestMagSq = diff.mag2();
        bestHit = h;
        extPos0 = intersection.position;
      }
    } else {
      // Accept a nearby hit in adjacent sector
      if (std::fabs(dn) > 50.0) continue;
      int sector = hit->getSector() - 1;
      int dSector = abs(intersection.sector - sector);
      if ((dSector != +1) && (dSector != m_BarrelNSector - 1)) continue;
      // Use the normal vector of the adjacent (hit's) sector
      G4ThreeVector nHit(m_BarrelSectorPerp[sector]);
      int fb = (intersection.isForward ? BKLM_FORWARD : BKLM_BACKWARD) - 1;
      double dn2 = intersection.position * nHit - m_BarrelModuleMiddleRadius[fb][sector][intersection.layer];
      dn = diff * nHit + dn2;
      if (std::fabs(dn) > 1.0) continue;
      // Project extrapolated track to the hit's plane in the adjacent sector
      G4ThreeVector extDir(intersection.momentum.unit());
      double extDirA = extDir * nHit;
      if (std::fabs(extDirA) < 1.0E-6) continue;
      G4ThreeVector projection = extDir * (dn2 / extDirA);
      if (projection.mag() > 15.0) continue;
      diff += projection - nHit * dn;
      if (diff.mag2() < diffBestMagSq) {
        diffBestMagSq = diff.mag2();
        bestHit = h;
        extPos0 = intersection.position - projection;
      }
    }
  }

  if (bestHit >= 0) {
    BKLMHit2d* hit = bklmHits[bestHit];
    intersection.isForward = hit->isForward();
    intersection.sector = hit->getSector() - 1;
    intersection.time = hit->getTime();
    double localVariance[2] = {m_BarrelScintVariance, m_BarrelScintVariance};
    if (hit->inRPC()) {
      int nStrips = hit->getPhiStripMax() - hit->getPhiStripMin() + 1;
      double dn = nStrips - 1.5;
      double factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.60; // measured-in-Belle resolution
      localVariance[0] = m_BarrelPhiStripVariance[intersection.layer] * factor;
      nStrips = hit->getZStripMax() - hit->getZStripMin() + 1;
      dn = nStrips - 1.5;
      factor = std::pow((0.9 + 0.4 * dn * dn), 1.5) * 0.55; // measured-in-Belle resolution
      localVariance[1] = m_BarrelZStripVariance[intersection.layer] * factor;
    }
    G4ThreeVector hitPos(hit->getGlobalPositionX(), hit->getGlobalPositionY(), hit->getGlobalPositionZ());
    adjustIntersection(intersection, localVariance, hitPos, extPos0);
    if (intersection.chi2 >= 0.0) {
      intersection.hit = bestHit;
      hit->isOnTrack(true);
      if (track != NULL) {
        track->addRelationTo(hit);
        RecoTrack* recoTrack = track->getRelatedTo<RecoTrack>();
        recoTrack->addBKLMHit(hit, recoTrack->getNumberOfTotalHits() + 1);
      }
    }
  }
  return intersection.chi2 >= 0.0;

}

bool TrackExtrapolateG4e::findMatchingEndcapHit(Intersection& intersection, const Track* track)
{

  StoreArray<EKLMHit2d> eklmHits(*m_EKLMHitsColName);
  double diffBestMagSq = 1.0E60;
  int bestHit = -1;
  int matchingLayer = intersection.layer + 1;
  int matchingEndcap = (intersection.isForward ? 2 : 1);
  G4ThreeVector n(0.0, 0.0, (intersection.isForward ? 1.0 : -1.0));
  for (int h = 0; h < eklmHits.getEntries(); ++h) {
    EKLMHit2d* hit = eklmHits[h];
    if (hit->getLayer() != matchingLayer) continue;
    if (hit->getEndcap() != matchingEndcap) continue;
    // DIVOT no such function for EKLM!
    // if (hit->isOutOfTime()) continue;
    if (std::fabs(hit->getTime() - m_MeanDt) > m_MaxDt) continue;
    G4ThreeVector diff(hit->getPositionX() - intersection.position.x(),
                       hit->getPositionY() - intersection.position.y(),
                       hit->getPositionZ() - intersection.position.z());
    double dn = diff * n; // in cm
    if (std::fabs(dn) > 2.0) continue;
    diff -= n * dn;
    if (diff.mag2() < diffBestMagSq) {
      diffBestMagSq = diff.mag2();
      bestHit = h;
    }
  }

  if (bestHit >= 0) {
    EKLMHit2d* hit = eklmHits[bestHit];
    intersection.hit = bestHit;
    intersection.isForward = (hit->getEndcap() == 2);
    intersection.sector = hit->getSector() - 1;
    intersection.time = hit->getTime();
    double localVariance[2] = {m_EndcapScintVariance, m_EndcapScintVariance};
    G4ThreeVector hitPos(hit->getPositionX(), hit->getPositionY(), hit->getPositionZ());
    adjustIntersection(intersection, localVariance, hitPos, intersection.position);
    if (intersection.chi2 >= 0.0) {
      // DIVOT no such function for EKLM!
      // hit->isOnTrack(true);
      if (track != NULL) {
        RelationVector<EKLMAlignmentHit> eklmAlignmentHits = hit->getRelationsFrom<EKLMAlignmentHit>();
        track->addRelationTo(hit);
        RecoTrack* recoTrack = track->getRelatedTo<RecoTrack>();
        for (unsigned int i = 0; i < eklmAlignmentHits.size(); ++i) {
          recoTrack->addEKLMHit(eklmAlignmentHits[i], recoTrack->getNumberOfTotalHits() + 1);
        }
      }
    }
  }
  return intersection.chi2 >= 0.0;

}

void TrackExtrapolateG4e::adjustIntersection(Intersection& intersection, const double localVariance[2],
                                             const G4ThreeVector& hitPos, const G4ThreeVector& extPos0)
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

// In most cases, extPos0 is the same as intersection.position.  They differ only when
// the nearest BKLM hit is in the sector adjacent to that of intersection.position.

  G4ThreeVector extPos(extPos0);
  G4ThreeVector extMom(intersection.momentum);
  G4ThreeVector extDir(extMom.unit());
  G4ThreeVector diffPos(hitPos - extPos);
  G4ErrorSymMatrix extCov(intersection.covariance);

// Track parameters (x,y,z,px,py,pz) before correction

  G4ErrorMatrix extPar(6, 1); // initialized to all zeroes
  extPar[0][0] = extPos.x();
  extPar[1][0] = extPos.y();
  extPar[2][0] = extPos.z();
  extPar[3][0] = extMom.x();
  extPar[4][0] = extMom.y();
  extPar[5][0] = extMom.z();

  G4ThreeVector nA;  // unit vector normal to the readout plane
  G4ThreeVector nB;  // unit vector along phi- or x-readout direction (for barrel or endcap)
  G4ThreeVector nC;  // unit vector along z- or y-readout direction (for barrel or endcap)
  if (intersection.inBarrel) {
    nA = m_BarrelSectorPerp[intersection.sector];
    nB = m_BarrelSectorPhi[intersection.sector];
    nC = G4ThreeVector(0.0, 0.0, 1.0);
  } else {
    double out = (intersection.isForward ? 1.0 : -1.0);
    nA = G4ThreeVector(0.0, 0.0, out);
    nB = G4ThreeVector(out, 0.0, 0.0);
    nC = G4ThreeVector(0.0, out, 0.0);
  }

// Don't adjust the extrapolation if the track is nearly tangent to the readout plane.

  double extDirA = extDir * nA;
  if (std::fabs(extDirA) < 1.0E-6) return;
  double extDirBA = extDir * nB / extDirA;
  double extDirCA = extDir * nC / extDirA;

// Move the extrapolated coordinate (at most a tiny amount!) to the plane of the hit.
// If the moved point is outside the KLM, don't do Kalman filtering.

  G4ThreeVector move = extDir * ((diffPos * nA) / extDirA);
  extPos += move;
  diffPos -= move;
  intersection.positionAtHitPlane = extPos;

// Projection jacobian onto the nB-nC measurement plane

  G4ErrorMatrix jacobian(2, 6); // initialized to all zeroes
  jacobian[0][0] = nB.x()  - nA.x() * extDirBA;
  jacobian[0][1] = nB.y()  - nA.y() * extDirBA;
  jacobian[0][2] = nB.z()  - nA.z() * extDirBA;
  jacobian[1][0] = nC.x()  - nA.x() * extDirCA;
  jacobian[1][1] = nC.y()  - nA.y() * extDirCA;
  jacobian[1][2] = nC.z()  - nA.z() * extDirCA;

// Residuals of EXT track and KLM hit on the nB-nC measurement plane

  G4ErrorMatrix residual(2, 1); // initialized to all zeroes
  residual[0][0] = diffPos.x() * jacobian[0][0] + diffPos.y() * jacobian[0][1] + diffPos.z() * jacobian[0][2];
  residual[1][0] = diffPos.x() * jacobian[1][0] + diffPos.y() * jacobian[1][1] + diffPos.z() * jacobian[1][2];

// Measurement errors in the detector plane

  G4ErrorSymMatrix hitCov(2, 0); // initialized to all zeroes
  hitCov[0][0] = localVariance[0];
  hitCov[1][1] = localVariance[1];
  // No magnetic field: increase the hit uncertainty
  if (m_MagneticField == 0.0) {
    hitCov[0][0] *= 10.0;
    hitCov[1][1] *= 10.0;
  }

// Now get the correction matrix: combined covariance of EXT and KLM hit.
// 1st dimension = nB, 2nd dimension = nC.

  G4ErrorSymMatrix correction(extCov.similarity(jacobian) + hitCov);

// Ignore the best hit if it is too far from the extrapolated-track intersection in the hit's plane

  if (residual[0][0] * residual[0][0] > correction[0][0] * m_MaxDistSqInVariances) return;
  if (residual[1][0] * residual[1][0] > correction[1][1] * m_MaxDistSqInVariances) return;

  int fail = 0;
  correction.invert(fail);
  if (fail != 0) return;

// Matrix inversion succeeeded and is reasonable.
// Evaluate chi-squared increment assuming that the Kalman filter
// won't be able to adjust the extrapolated track's position (fall-back).

  intersection.chi2 = (correction.similarityT(residual))[0][0];

// Do the Kalman filtering

  G4ErrorMatrix gain((extCov * jacobian.T()) * correction);
  G4ErrorSymMatrix HRH(correction.similarityT(jacobian));

  extCov -= HRH.similarity(extCov);
  extPar += gain * residual;
  extPos.set(extPar[0][0], extPar[1][0], extPar[2][0]);
  extMom.set(extPar[3][0], extPar[4][0], extPar[5][0]);

// Calculate the chi-squared increment using the Kalman-filtered state

  correction = hitCov - extCov.similarity(jacobian);
  correction.invert(fail);
  if (fail != 0) return;

  diffPos = hitPos - extPos;
  residual[0][0] = diffPos.x() * jacobian[0][0] + diffPos.y() * jacobian[0][1] + diffPos.z() * jacobian[0][2];
  residual[1][0] = diffPos.x() * jacobian[1][0] + diffPos.y() * jacobian[1][1] + diffPos.z() * jacobian[1][2];
  intersection.chi2 = (correction.similarityT(residual))[0][0];

// Update the position, momentum and covariance of the point
// Project the corrected extrapolation to the plane of the original
// extrapolation's intersection.position. (Note: intersection.position is the same as
// extPos0 in all cases except when nearest BKLM hit is in adjacent
// sector, for which extPos0 is a projected position to the hit's plane.)
// Also, leave the momentum magnitude unchanged.

  intersection.position = extPos + extDir * (((intersection.position - extPos) * nA) / extDirA);
  intersection.momentum = intersection.momentum.mag() * extMom.unit();
  intersection.covariance = extCov;

}

void TrackExtrapolateG4e::finishTrack(const ExtState& extState, Muid* muid, bool isForward)
{

  // Done with this track: compute likelihoods and fill the muid object

  int lastExtLayer(extState.lastBarrelExtLayer + extState.lastEndcapExtLayer + 1);
  // outcome: 0=didn't reach KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit
  int outcome(0);
  if ((extState.lastBarrelExtLayer >= 0) || (extState.lastEndcapExtLayer >= 0)) {
    outcome = ((extState.lastEndcapExtLayer < 0) ? 1 : 2) + (extState.escaped ? 2 : 0);
  }

  muid->setOutcome(outcome);
  muid->setBarrelExtLayer(extState.lastBarrelExtLayer);
  muid->setEndcapExtLayer(extState.lastEndcapExtLayer);
  muid->setBarrelHitLayer(extState.lastBarrelHitLayer);
  muid->setEndcapHitLayer(extState.lastEndcapHitLayer);
  muid->setExtLayer(lastExtLayer);
  muid->setHitLayer(((extState.lastEndcapHitLayer == -1) ?
                     extState.lastBarrelHitLayer :
                     extState.lastBarrelExtLayer + extState.lastEndcapHitLayer + 1));
  muid->setChiSquared(extState.chi2);
  muid->setDegreesOfFreedom(extState.nPoint);
  muid->setExtLayerPattern(extState.extLayerPattern);
  muid->setHitLayerPattern(extState.hitLayerPattern);

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
    int charge = (muid->getPDGCode() > 0);
    if ((abs(muid->getPDGCode()) == Const::muon.getPDGCode()) ||
        (abs(muid->getPDGCode()) == Const::electron.getPDGCode())) charge = -charge;
    if (charge > 0) {
      muon = m_MuonPlusPar->getPDF(muid, isForward);
      pion = m_PionPlusPar->getPDF(muid, isForward);
      kaon = m_KaonPlusPar->getPDF(muid, isForward);
      proton = m_ProtonPar->getPDF(muid, isForward);
      deuteron = m_DeuteronPar->getPDF(muid, isForward);
      electron = m_PositronPar->getPDF(muid, isForward);
    } else {
      muon = m_MuonMinusPar->getPDF(muid, isForward);
      pion = m_PionMinusPar->getPDF(muid, isForward);
      kaon = m_KaonMinusPar->getPDF(muid, isForward);
      proton = m_AntiprotonPar->getPDF(muid, isForward);
      deuteron = m_AntideuteronPar->getPDF(muid, isForward);
      electron = m_ElectronPar->getPDF(muid, isForward);
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
