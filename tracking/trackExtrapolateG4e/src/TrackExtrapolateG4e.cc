/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <tracking/trackExtrapolateG4e/TrackExtrapolateG4e.h>

/* Belle 2 headers. */
#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/logging/Logger.h>
#include <genfit/Exception.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMStatus.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/dataobjects/KLMMuidHit.h>
#include <klm/dataobjects/eklm/EKLMAlignmentHit.h>
#include <klm/muid/MuidBuilder.h>
#include <klm/muid/MuidElementNumbers.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <simulation/kernel/ExtManager.h>

/* CLHEP headers. */
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

/* Geant4 headers. */
#include <G4ErrorFreeTrajState.hh>
#include <G4ErrorMatrix.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorSymMatrix.hh>
#include <G4ParticleTable.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4Point3D.hh>
#include <G4StateManager.hh>
#include <G4Step.hh>
#include <G4StepPoint.hh>
#include <G4ThreeVector.hh>
#include <G4TouchableHandle.hh>
#include <G4Track.hh>
#include <G4UImanager.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VTouchable.hh>

/* C++ headers. */
#include <algorithm>
#include <cmath>
#include <iostream>

#define TWOPI (2.0*M_PI)
#define PI_8 (0.125*M_PI)
#define DEPTH_RPC 9
#define DEPTH_SCINT 11

using namespace Belle2;

TrackExtrapolateG4e* TrackExtrapolateG4e::m_Singleton = nullptr;

TrackExtrapolateG4e* TrackExtrapolateG4e::getInstance()
{
  if (m_Singleton == nullptr)
    m_Singleton = new TrackExtrapolateG4e;
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
  m_ExtMgr(nullptr), // initialized later
  m_HypothesesExt(nullptr), // initialized later
  m_HypothesesMuid(nullptr), // initialized later
  m_DefaultHypotheses(nullptr), // initialized later
  m_EnterExit(nullptr), // initialized later
  m_BKLMVolumes(nullptr), // initialized later
  m_TargetExt(nullptr), // initialized later
  m_TargetMuid(nullptr), // initialized later
  m_MinRadiusSq(0.0), // initialized later
  m_OffsetZ(0.0), // initialized later
  m_BarrelNSector(0), // initialized later
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
  m_eklmTransformData(nullptr) // initialized later
{
  for (int j = 0; j < BKLMElementNumbers::getMaximalLayerNumber() + 1; ++j) {
    m_BarrelPhiStripVariance[j] = 0.0;
    m_BarrelZStripVariance[j] = 0.0;
    m_BarrelPhiStripVariance[j] = 0.0;
    m_EndcapModuleMiddleZ[j] = 0.0;
  }
  for (int s = 0; s < BKLMElementNumbers::getMaximalSectorNumber() + 1; ++s) {
    for (int j = 0; j < BKLMElementNumbers::getMaximalLayerNumber() + 1; ++j) {
      m_BarrelModuleMiddleRadius[0][s][j] = 0.0;
      m_BarrelModuleMiddleRadius[1][s][j] = 0.0;
    }
    m_BarrelSectorPerp[s] = G4ThreeVector(0.0, 0.0, 0.0);
    m_BarrelSectorPhi[s] = G4ThreeVector(0.0, 0.0, 0.0);
  }
  m_klmElementNumbers = &(KLMElementNumbers::Instance());
  m_eklmElementNumbers = &(EKLMElementNumbers::Instance());
}

TrackExtrapolateG4e::~TrackExtrapolateG4e()
{
}

// Initialize for EXT
void TrackExtrapolateG4e::initialize(double minPt, double minKE,
                                     std::vector<Const::ChargedStable>& hypotheses)
{
  m_ExtInitialized = true;

  // Define required objects, register the new ones and relations
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_extHits.registerInDataStore();
  m_tracks.registerRelationTo(m_extHits);

  // Save the magnetic field z component (gauss) at the origin
  m_MagneticField = BFieldManager::getField(B2Vector3D(0, 0, 0)).Z() / Unit::T * CLHEP::tesla / CLHEP::gauss;

  // Convert user cutoff values to geant4 units
  m_MinPt = std::max(0.0, minPt) * CLHEP::GeV;
  m_MinKE = std::max(0.0, minKE) * CLHEP::GeV;

  // Save pointer to the list of particle hypotheses for EXT extrapolation
  m_HypothesesExt = &hypotheses;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Store the address of the ExtManager (used later)
  m_ExtMgr = Simulation::ExtManager::GetManager();

  // Set up the EXT-specific geometry (might have already been done by MUID)
  if (m_TargetExt == nullptr) {
    if (!m_COILGeometryPar.isValid())
      B2FATAL("Coil geometry data are not available.");
    double offsetZ = m_COILGeometryPar->getGlobalOffsetZ();
    double rMinCoil = m_COILGeometryPar->getCryoRmin();
    double halfLength = m_COILGeometryPar->getCryoLength();
    m_COILGeometryPar.addCallback([this, &offsetZ, &rMinCoil, &halfLength]() {
      offsetZ = m_COILGeometryPar->getGlobalOffsetZ();
      rMinCoil = m_COILGeometryPar->getCryoRmin();
      halfLength = m_COILGeometryPar->getCryoLength();
    });
    m_TargetExt = new Simulation::ExtCylSurfaceTarget(rMinCoil, offsetZ - halfLength, offsetZ + halfLength);
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetExt);
  }
  if (!m_BeamPipeGeo.isValid())
    B2FATAL("Beam pipe geometry data are not available.");
  double beampipeRadius = m_BeamPipeGeo->getParameter("Lv2OutBe.R2") * CLHEP::cm; // mm
  m_BeamPipeGeo.addCallback([this, &beampipeRadius]() {
    beampipeRadius = m_BeamPipeGeo->getParameter("Lv2OutBe.R2") * CLHEP::cm;
  });
  m_MinRadiusSq = beampipeRadius * beampipeRadius; // mm^2
}

// Initialize for MUID
void TrackExtrapolateG4e::initialize(double meanDt, double maxDt, double maxKLMTrackHitDistance,
                                     double maxKLMTrackClusterDistance, double maxECLTrackClusterDistance,
                                     double minPt, double minKE, bool addHitsToRecoTrack,
                                     std::vector<Const::ChargedStable>& hypotheses)
{
  m_MuidInitialized = true;
  m_addHitsToRecoTrack = addHitsToRecoTrack;

  // Define required objects, register the new ones and relations
  m_eclClusters.isRequired();
  m_bklmHit2ds.isRequired();
  m_eklmHit2ds.isRequired();
  m_klmClusters.isRequired();
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_extHits.registerInDataStore();
  m_klmMuidLikelihoods.registerInDataStore();
  m_klmMuidHits.registerInDataStore();
  m_trackClusterSeparations.registerInDataStore();
  m_tracks.registerRelationTo(m_extHits);
  m_tracks.registerRelationTo(m_klmMuidLikelihoods);
  m_tracks.registerRelationTo(m_klmMuidHits);
  m_tracks.registerRelationTo(m_bklmHit2ds);
  m_tracks.registerRelationTo(m_eklmHit2ds);
  m_tracks.registerRelationTo(m_trackClusterSeparations);
  m_tracks.registerRelationTo(m_klmClusters);
  m_klmClusters.registerRelationTo(m_trackClusterSeparations);
  m_eclClusters.registerRelationTo(m_extHits);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  // Save the in-time cut's central value and width for valid hits
  m_MeanDt = meanDt;
  m_MaxDt = maxDt;

  // Save the magnetic field z component (gauss) at the origin
  m_MagneticField = BFieldManager::getField(B2Vector3D(0, 0, 0)).Z() / Unit::T * CLHEP::tesla / CLHEP::gauss;

  // Convert from sigma to variance for hit-position uncertainty
  m_MaxDistSqInVariances = maxKLMTrackHitDistance * maxKLMTrackHitDistance;

  // Convert user cutoff values to geant4 units
  m_MaxKLMTrackClusterDistance = std::max(0.0, maxKLMTrackClusterDistance) * CLHEP::cm;
  m_MaxECLTrackClusterDistance = std::max(0.0, maxECLTrackClusterDistance) * CLHEP::cm;
  m_MinPt = std::max(0.0, minPt) * CLHEP::GeV;
  m_MinKE = std::max(0.0, minKE) * CLHEP::GeV;

  // Save pointer to the list of particle hypotheses for EXT extrapolation
  m_HypothesesMuid = &hypotheses;

  // Define the list of volumes that will have their entry and/or
  // exit points stored during the extrapolation.
  registerVolumes();

  // Store the address of the ExtManager (used later)
  m_ExtMgr = Simulation::ExtManager::GetManager();

  // Set up the EXT-specific geometry (might have already been done by EXT)
  if (m_TargetExt == nullptr) {
    if (!m_COILGeometryPar.isValid())
      B2FATAL("Coil geometry data are not available.");
    double offsetZ = m_COILGeometryPar->getGlobalOffsetZ();
    double rMinCoil = m_COILGeometryPar->getCryoRmin();
    double halfLength = m_COILGeometryPar->getCryoLength();
    m_COILGeometryPar.addCallback([this, &offsetZ, &rMinCoil, &halfLength]() {
      offsetZ = m_COILGeometryPar->getGlobalOffsetZ();
      rMinCoil = m_COILGeometryPar->getCryoRmin();
      halfLength = m_COILGeometryPar->getCryoLength();
    });
    m_TargetExt = new Simulation::ExtCylSurfaceTarget(rMinCoil, offsetZ - halfLength, offsetZ + halfLength);
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetExt);
  }
  if (!m_BeamPipeGeo.isValid())
    B2FATAL("Beam pipe geometry data are not available.");
  double beampipeRadius = m_BeamPipeGeo->getParameter("Lv2OutBe.R2") * CLHEP::cm; // mm
  m_BeamPipeGeo.addCallback([this, &beampipeRadius]() {
    beampipeRadius = m_BeamPipeGeo->getParameter("Lv2OutBe.R2") * CLHEP::cm;
  });
  m_MinRadiusSq = beampipeRadius * beampipeRadius; // mm^2

  // Set up the MUID-specific geometry
  bklm::GeometryPar* bklmGeometry = bklm::GeometryPar::instance();
  const EKLM::GeometryData& eklmGeometry = EKLM::GeometryData::Instance();
  m_BarrelHalfLength = bklmGeometry->getHalfLength() * CLHEP::cm; // in G4 units (mm)
  m_EndcapHalfLength = 0.5 * eklmGeometry.getSectionPosition()->getLength(); // in G4 units (mm)
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
  m_EndcapMinR = eklmGeometry.getSectionPosition()->getInnerR() / CLHEP::cm; // in G4e units (cm)
  m_EndcapMaxR = eklmGeometry.getSectionPosition()->getOuterR() / CLHEP::cm; // in G4e units (cm)
  m_EndcapMiddleZ = m_BarrelHalfLength + m_EndcapHalfLength;                // in G4e units (cm)

  // Measurement uncertainties and acceptance windows
  double width = eklmGeometry.getStripGeometry()->getWidth() / CLHEP::cm; // in G4e units (cm)
  m_EndcapScintVariance = width * width / 12.0;
  width = bklmGeometry->getScintHalfWidth() * 2.0;                        // in G4e units (cm)
  m_BarrelScintVariance = width * width / 12.0;
  for (int layer = 1; layer <= BKLMElementNumbers::getMaximalLayerNumber(); ++layer) {
    const bklm::Module* module =
      bklmGeometry->findModule(BKLMElementNumbers::c_ForwardSection, 1, layer);
    width = module->getPhiStripWidth(); // in G4e units (cm)
    m_BarrelPhiStripVariance[layer - 1] = width * width / 12.0;
    width = module->getZStripWidth(); // in G4e units (cm)
    m_BarrelZStripVariance[layer - 1] = width * width / 12.0;
  }

  // KLM geometry (for associating KLM hit with extrapolated crossing point)
  m_OutermostActiveBarrelLayer = BKLMElementNumbers::getMaximalLayerNumber() - 1; // zero-based counting
  for (int sector = 1; sector <= BKLMElementNumbers::getMaximalSectorNumber(); ++sector) {
    double phi = M_PI_4 * (sector - 1);
    m_BarrelSectorPerp[sector - 1].set(std::cos(phi), std::sin(phi), 0.0);
    m_BarrelSectorPhi[sector - 1].set(-std::sin(phi), std::cos(phi), 0.0);
  }
  KLMChannelIndex klmLayers(KLMChannelIndex::c_IndexLevelLayer);
  for (KLMChannelIndex& klmLayer : klmLayers) {
    if (klmLayer.getSubdetector() == KLMElementNumbers::c_BKLM)
      m_BarrelModuleMiddleRadius[klmLayer.getSection()][klmLayer.getSector() - 1][klmLayer.getLayer() - 1] =
        bklmGeometry->getActiveMiddleRadius(klmLayer.getSection(), klmLayer.getSector(), klmLayer.getLayer()); // in G4e units (cm)
  }
  double dz(eklmGeometry.getLayerShiftZ() / CLHEP::cm); // in G4e units (cm)
  double z0((eklmGeometry.getSectionPosition()->getZ()
             + eklmGeometry.getLayerShiftZ()
             - 0.5 * eklmGeometry.getSectionPosition()->getLength()
             - 0.5 * eklmGeometry.getLayerPosition()->getLength()) / CLHEP::cm); // in G4e units (cm)
  m_OutermostActiveForwardEndcapLayer = m_eklmElementNumbers->getMaximalDetectorLayerNumber(EKLMElementNumbers::c_ForwardSection) -
                                        1; // zero-based counting
  m_OutermostActiveBackwardEndcapLayer = m_eklmElementNumbers->getMaximalDetectorLayerNumber(EKLMElementNumbers::c_BackwardSection) -
                                         1; // zero-based counting
  for (int layer = 1; layer <= EKLMElementNumbers::getMaximalLayerNumber(); ++layer) {
    m_EndcapModuleMiddleZ[layer - 1] = z0 + dz * (layer - 1); // in G4e units (cm)
  }

  m_eklmTransformData = &(EKLM::TransformDataGlobalAligned::Instance());
}

void TrackExtrapolateG4e::beginRun(bool byMuid)
{
  B2DEBUG(20, (byMuid ? "muid" : "ext"));
  if (byMuid) {
    if (!m_klmChannelStatus.isValid())
      B2FATAL("KLM channel status data are not available.");
    if (!m_klmStripEfficiency.isValid())
      B2FATAL("KLM strip efficiency data are not available.");
    if (!m_klmLikelihoodParameters.isValid())
      B2FATAL("KLM likelihood parameters are not available.");
    std::vector<int> muidPdgCodes = MuidElementNumbers::getPDGVector();
    if (!m_MuidBuilderMap.empty()) {
      if (m_klmLikelihoodParameters.hasChanged()) { /* Clear m_MuidBuilderMap if KLMLikelihoodParameters payload changed. */
        for (auto const& [pdg, muidBuilder] : m_MuidBuilderMap)
          delete muidBuilder;
        m_MuidBuilderMap.clear();
      } else /* Return if m_MuidBuilderMap is already initialized. */
        return;
    }
    for (int pdg : muidPdgCodes)
      m_MuidBuilderMap.insert(std::pair<int, MuidBuilder*>(pdg, new MuidBuilder(pdg)));
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

  // Loop over the reconstructed tracks
  // Do extrapolation for each hypothesis of each reconstructed track.
  if (byMuid) { // event() called by Muid module
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetMuid);
    std::vector<std::pair<ECLCluster*, G4ThreeVector> > eclClusterInfo(m_eclClusters.getEntries());
    for (int c = 0; c < m_eclClusters.getEntries(); ++c) {
      eclClusterInfo[c].first = m_eclClusters[c];
      eclClusterInfo[c].second = G4ThreeVector(m_eclClusters[c]->getClusterPosition().X(),
                                               m_eclClusters[c]->getClusterPosition().Y(),
                                               m_eclClusters[c]->getClusterPosition().Z()) * CLHEP::cm;
    }
    std::vector<std::pair<KLMCluster*, G4ThreeVector> > klmClusterInfo(m_klmClusters.getEntries());
    for (int c = 0; c < m_klmClusters.getEntries(); ++c) {
      klmClusterInfo[c].first = m_klmClusters[c];
      klmClusterInfo[c].second = G4ThreeVector(m_klmClusters[c]->getClusterPosition().X(),
                                               m_klmClusters[c]->getClusterPosition().Y(),
                                               m_klmClusters[c]->getClusterPosition().Z()) * CLHEP::cm;
    }
    // Keep track of (re-)use of BKLMHit2ds
    std::vector<std::map<const Track*, double> > bklmHitUsed(m_bklmHit2ds.getEntries());
    for (auto& b2track : m_tracks) {
      for (const auto& hypothesis : *m_HypothesesMuid) {
        int pdgCode = hypothesis.getPDGCode();
        if (hypothesis == Const::electron || hypothesis == Const::muon)
          pdgCode = -pdgCode;
        G4ErrorFreeTrajState g4eState("g4e_mu+", G4ThreeVector(), G4ThreeVector()); // will be updated
        ExtState extState = getStartPoint(b2track, pdgCode, g4eState);
        swim(extState, g4eState, &eclClusterInfo, &klmClusterInfo, &bklmHitUsed);
      } // Muid hypothesis loop
    } // Muid track loop
  } else { // event() called by Ext module
    G4ErrorPropagatorData::GetErrorPropagatorData()->SetTarget(m_TargetExt);
    for (auto& b2track : m_tracks) {
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
  if (m_DefaultHypotheses != nullptr)
    delete m_DefaultHypotheses;
  if (byMuid) {
    delete m_TargetMuid;
    for (auto const& [pdg, muidBuilder] : m_MuidBuilderMap)
      delete muidBuilder;
  }
  if (m_TargetExt != nullptr) {
    delete m_TargetExt;
    m_TargetExt = nullptr;
  }
  if (m_EnterExit != nullptr) {
    delete m_EnterExit;
    delete m_BKLMVolumes;
    m_ExtMgr->RunTermination();
    m_EnterExit = nullptr;
    m_BKLMVolumes = nullptr;
  }
}

void TrackExtrapolateG4e::extrapolate(int pdgCode, // signed for charge
                                      double tof, // in ns (from IP to position)
                                      // DIVOT bool isCosmic, // true for back-extrapolation
                                      const G4ThreeVector& position, // in cm (genfit2 units)
                                      const G4ThreeVector& momentum, // in GeV/c (genfit2 units)
                                      const G4ErrorSymMatrix& covariance) // (6x6) using cm, GeV/c (genfit2 units)
{
  bool isCosmic = false; // DIVOT
  if ((!m_ExtInitialized) && (!m_MuidInitialized)) {
    // No EXT nor MUID module in analysis path ==> mimic ext::initialize() with reasonable defaults.
    // The default values are taken from the EXT module's parameter definitions.
    Simulation::ExtManager* extMgr = Simulation::ExtManager::GetManager();
    extMgr->Initialize("Ext", "default", 0.0, 0.25, false, 0, std::vector<std::string>());
    // Redefine geant4e step length, magnetic field step limitation (fraction of local curvature radius),
    // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
    // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
    // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/stepLength 250 mm");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");
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
  // cppcheck-suppress knownConditionTrueFalse
  if (isCosmic)
    momentumG4e = -momentumG4e;
  G4ErrorSymMatrix covarianceG4e(5, 0); // in Geant4e units (GeV/c, cm)
  fromPhasespaceToG4e(momentum, covariance, covarianceG4e);
  G4String nameG4e("g4e_" + G4ParticleTable::GetParticleTable()->FindParticle(pdgCode)->GetParticleName());
  G4ErrorFreeTrajState g4eState(nameG4e, positionG4e, momentumG4e, covarianceG4e);
  ExtState extState = { nullptr, pdgCode, isCosmic, tof, 0.0,                         // for EXT and MUID
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
    extMgr->Initialize("Muid", "default", 0.0, 0.25, false, 0, std::vector<std::string>());
    // Redefine geant4e step length, magnetic field step limitation (fraction of local curvature radius),
    // and kinetic energy loss limitation (maximum fractional energy loss) by communicating with
    // the geant4 UI.  (Commands were defined in ExtMessenger when physics list was set up.)
    // *NOTE* If module muid runs after this, its G4UImanager commands will override these.
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/stepLength 250 mm");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/magField 0.001");
    G4UImanager::GetUIpointer()->ApplyCommand("/geant4e/limits/energyLoss 0.05");
    m_DefaultHypotheses = new std::vector<Const::ChargedStable>; // not used
    initialize(0.0, 30.0, 3.5, 150.0, 100.0, 0.1, 0.002, false, *m_DefaultHypotheses);
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
  if (isCosmic)
    momentumG4e = -momentumG4e;
  G4ErrorSymMatrix covarianceG4e(5, 0); // in Geant4e units (GeV/c, cm)
  fromPhasespaceToG4e(momentum, covariance, covarianceG4e);
  G4String nameG4e("g4e_" + G4ParticleTable::GetParticleTable()->FindParticle(pdgCode)->GetParticleName());
  G4ErrorFreeTrajState g4eState(nameG4e, positionG4e, momentumG4e, covarianceG4e);
  ExtState extState = { nullptr, pdgCode, isCosmic, tof, 0.0,                             // for EXT and MUID
                        momentumG4e.unit(), 0.0, 0, 0, 0, -1, -1, -1, -1, 0, 0, false  // for MUID only
                      };
  swim(extState, g4eState, nullptr, nullptr, nullptr);
}

// Swim one track for MUID until it stops or leaves the KLM-bounding cylinder
void TrackExtrapolateG4e::swim(ExtState& extState, G4ErrorFreeTrajState& g4eState,
                               const std::vector<std::pair<ECLCluster*, G4ThreeVector> >* eclClusterInfo,
                               const std::vector<std::pair<KLMCluster*, G4ThreeVector> >* klmClusterInfo,
                               std::vector<std::map<const Track*, double> >* bklmHitUsed)
{
  if (extState.pdgCode == 0)
    return;
  if (g4eState.GetMomentum().perp() <= m_MinPt)
    return;
  if (m_TargetMuid->GetDistanceFromPoint(g4eState.GetPosition()) < 0.0)
    return;
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(extState.pdgCode);
  double mass = particle->GetPDGMass();
  double minPSq = (mass + m_MinKE) * (mass + m_MinKE) - mass * mass;
  // Create structures for ECLCluster-Track matching
  std::vector<double> eclClusterDistance;
  std::vector<ExtHit> eclHit1, eclHit2, eclHit3;
  if (eclClusterInfo != nullptr) {
    eclClusterDistance.resize(eclClusterInfo->size(), 1.0E10); // "positive infinity"
    ExtHit tempExtHit(extState.pdgCode, Const::EDetector::ECL, 0, EXT_FIRST,
                      extState.isCosmic, 0.0,
                      G4ThreeVector(), G4ThreeVector(), G4ErrorSymMatrix(6));
    eclHit1.resize(eclClusterInfo->size(), tempExtHit);
    eclHit2.resize(eclClusterInfo->size(), tempExtHit);
    eclHit3.resize(eclClusterInfo->size(), tempExtHit);
  }
  // Create structures for KLMCluster-Track matching
  std::vector<TrackClusterSeparation> klmHit;
  if (klmClusterInfo != nullptr) {
    klmHit.resize(klmClusterInfo->size()); // initialize each to huge distance
  }
  KLMMuidLikelihood* klmMuidLikelihood = m_klmMuidLikelihoods.appendNew(); // rest of this object will be filled later
  klmMuidLikelihood->setPDGCode(extState.pdgCode);
  if (extState.track != nullptr)
    extState.track->addRelationTo(klmMuidLikelihood);
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
      if (createMuidHit(extState, g4eState, klmMuidLikelihood, bklmHitUsed)) {
        // Force geant4e to update its G4Track from the Kalman-updated state
        m_ExtMgr->GetPropagator()->SetStepN(0);
      }
      if (eclClusterInfo != nullptr) {
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
      if (klmClusterInfo != nullptr) {
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

  finishTrack(extState, klmMuidLikelihood, (g4eState.GetPosition().z() > m_OffsetZ));

  if (eclClusterInfo != nullptr) {
    for (unsigned int c = 0; c < eclClusterInfo->size(); ++c) {
      if (eclHit1[c].getStatus() != EXT_FIRST) {
        ExtHit* h = m_extHits.appendNew(eclHit1[c]);
        (*eclClusterInfo)[c].first->addRelationTo(h);
        extState.track->addRelationTo(h);
      }
      if (eclHit2[c].getStatus() != EXT_FIRST) {
        ExtHit* h = m_extHits.appendNew(eclHit2[c]);
        (*eclClusterInfo)[c].first->addRelationTo(h);
        extState.track->addRelationTo(h);
      }
      if (eclHit3[c].getStatus() != EXT_FIRST) {
        ExtHit* h = m_extHits.appendNew(eclHit3[c]);
        (*eclClusterInfo)[c].first->addRelationTo(h);
        extState.track->addRelationTo(h);
      }
    }
  }

  if (klmClusterInfo != nullptr) {
    // here we set a relation only to the closest KLMCluster
    // and we don't set any relation if the distance is too large
    double minDistance = m_MaxKLMTrackClusterDistance;
    unsigned int closestCluster = 0;
    for (unsigned int c = 0; c < klmClusterInfo->size(); ++c) {
      if (klmHit[c].getDistance() > 1.0E9) {
        continue;
      }
      TrackClusterSeparation* h = m_trackClusterSeparations.appendNew(klmHit[c]);
      (*klmClusterInfo)[c].first->addRelationTo(h); // relation KLMCluster to TrackSep
      extState.track->addRelationTo(h); // relation Track to TrackSep
      if (klmHit[c].getDistance() < minDistance) {
        closestCluster = c;
        minDistance = klmHit[c].getDistance();
      }
    }
    if (minDistance < m_MaxKLMTrackClusterDistance) {
      // set the relation Track to KLMCluster, using the distance as weight
      extState.track->addRelationTo((*klmClusterInfo)[closestCluster].first, 1. / minDistance);
    }
  }

}

// Swim one track for EXT until it stops or leaves the ECL-bounding  cylinder
void TrackExtrapolateG4e::swim(ExtState& extState, G4ErrorFreeTrajState& g4eState)
{
  if (extState.pdgCode == 0)
    return;
  if (g4eState.GetMomentum().perp() <= m_MinPt)
    return;
  if (m_TargetExt->GetDistanceFromPoint(g4eState.GetPosition()) < 0.0)
    return;
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
    if (extState.isCosmic)
      mom = -mom;
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
  if (m_EnterExit != nullptr) // Only do this once
    return;
  m_EnterExit = new std::map<G4VPhysicalVolume*, enum VolTypes>;
  m_BKLMVolumes = new std::vector<G4VPhysicalVolume*>;
  for (std::vector<G4VPhysicalVolume*>::iterator iVol = pvStore->begin();
       iVol != pvStore->end(); ++iVol) {
    const G4String name = (*iVol)->GetName();
    // Do not store ExtHits in CDC, so let's start from TOP and ARICH.
    // TOP doesn't have one envelope; it has 16 "TOPModule"s
    if (name.find("TOPModule") != std::string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_TOP1;
    }
    // TOP quartz bar (=sensitive)
    else if (name.find("_TOPPrism_") != std::string::npos ||
             name.find("_TOPBarSegment") != std::string::npos ||
             name.find("_TOPMirrorSegment") != std::string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_TOP2;
    }
    // TOP quartz glue (not sensitive?)
    else if (name.find("TOPBarSegment1Glue") != std::string::npos ||
             name.find("TOPBarSegment2Glue") != std::string::npos ||
             name.find("TOPMirrorSegmentGlue") != std::string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_TOP3;
      // ARICH volumes
    } else if (name == "ARICH.AerogelSupportPlate") {
      (*m_EnterExit)[*iVol] = VOLTYPE_ARICH1;
    } else if (name == "ARICH.AerogelImgPlate") {
      (*m_EnterExit)[*iVol] = VOLTYPE_ARICH2;
    } else if (name.find("ARICH.HAPDWindow") != std::string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_ARICH3;
    }
    // ECL crystal
    else if (name.find("lv_barrel_crystal_") != std::string::npos ||
             name.find("lv_forward_crystal_") != std::string::npos ||
             name.find("lv_backward_crystal_") != std::string::npos) {
      (*m_EnterExit)[*iVol] = VOLTYPE_ECL;
    }
    // Barrel KLM: BKLM.Layer**GasPhysical for RPCs or BKLM.Layer**ChimneyGasPhysical for RPCs
    //             BKLM.ScintActiveType*Physical for scintillator strips
    else if (name.compare(0, 5, "BKLM.") == 0) {
      if (name.find("GasPhysical") != std::string::npos) {
        (*m_EnterExit)[*iVol] = VOLTYPE_BKLM1;
      } else if (name.find("ScintActiveType") != std::string::npos) {
        (*m_EnterExit)[*iVol] = VOLTYPE_BKLM2;
      } else if ((name.find("ScintType") != std::string::npos) ||
                 (name.find("ElectrodePhysical") != std::string::npos)) {
        m_BKLMVolumes->push_back(*iVol);
      }
    }
    // Endcap KLM: StripSensitive_*
    else if (name.compare(0, 14, "StripSensitive") == 0) {
      (*m_EnterExit)[*iVol] = VOLTYPE_EKLM;
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
  if (it == m_EnterExit->end())
    return;

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
      if (touch->GetHistoryDepth() >= 1)
        copyID = touch->GetVolume(1)->GetCopyNo();
      return;
    case VOLTYPE_TOP3:
      detID = Const::EDetector::TOP;
      if (touch->GetHistoryDepth() >= 2)
        copyID = touch->GetVolume(2)->GetCopyNo();
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
      if (touch->GetHistoryDepth() >= 2)
        copyID = touch->GetVolume(2)->GetCopyNo();
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
        int section = touch->GetCopyNumber(7);
        copyID = BKLMElementNumbers::moduleNumber(section, sector, layer);
      }
      return;
    case VOLTYPE_BKLM2: // BKLM scints
      detID = Const::EDetector::BKLM;
      if (touch->GetHistoryDepth() == DEPTH_SCINT) {
        int strip = touch->GetCopyNumber(1);
        int plane = (touch->GetCopyNumber(2) == BKLM_INNER) ?
                    BKLMElementNumbers::c_PhiPlane :
                    BKLMElementNumbers::c_ZPlane;
        int layer = touch->GetCopyNumber(6);
        int sector = touch->GetCopyNumber(8);
        int section = touch->GetCopyNumber(9);
        copyID = BKLMElementNumbers::channelNumber(
                   section, sector, layer, plane, strip);
        BKLMStatus::setMaximalStrip(copyID, strip);
      }
      return;
    case VOLTYPE_EKLM:
      detID = Const::EDetector::EKLM;
      copyID = EKLMElementNumbers::Instance().stripNumber(
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
  if (recoTrack == nullptr) {
    B2WARNING("Track without associated RecoTrack: skipping extrapolation for this track.");
    extState.pdgCode = 0; // prevent start of extrapolation in swim()
    return extState;
  }
  const genfit::AbsTrackRep* trackRep = recoTrack->getCardinalRepresentation();
  // check for a valid track fit
  if (!recoTrack->wasFitSuccessful(trackRep)) {
    B2WARNING("RecoTrack fit failed for cardinal representation: skipping extrapolation for this track.");
    extState.pdgCode = 0; // prevent start of extrapolation in swim()
    return extState;
  }
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
  } catch (const genfit::Exception&) {
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
  if (extState.isCosmic)
    mom = -mom;
  G4ErrorSymMatrix covariance(6, 0);
  fromG4eToPhasespace(g4eState, covariance);
  ExtHit* extHit = m_extHits.appendNew(extState.pdgCode, detID, copyID, status,
                                       extState.isCosmic, extState.tof,
                                       pos, mom, covariance);
  // If called standalone, there will be no associated track
  if (extState.track != nullptr)
    extState.track->addRelationTo(extHit);
}

// Write another volume-entry point on track.
// The track state will be modified here by the Kalman fitter.

bool TrackExtrapolateG4e::createMuidHit(ExtState& extState, G4ErrorFreeTrajState& g4eState, KLMMuidLikelihood* klmMuidLikelihood,
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
        float layerBarrelEfficiency = 1.;
        for (int plane = 0; plane <= BKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
          layerBarrelEfficiency *= m_klmStripEfficiency->getBarrelEfficiency(
                                     intersection.isForward ? BKLMElementNumbers::c_ForwardSection : BKLMElementNumbers::c_BackwardSection,
                                     intersection.sector + 1, intersection.layer + 1, plane, 1);
        }
        klmMuidLikelihood->setExtBKLMEfficiencyValue(intersection.layer, layerBarrelEfficiency);
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
        std::vector<G4VPhysicalVolume*>::iterator j = find(m_BKLMVolumes->begin(), m_BKLMVolumes->end(),
                                                           g4eState.GetG4Track()->GetVolume());
        if (j != m_BKLMVolumes->end()) {
          bool isDead = true; // by default, the nearest orthogonal strips are dead
          int section = intersection.isForward ?
                        BKLMElementNumbers::c_ForwardSection :
                        BKLMElementNumbers::c_BackwardSection;
          int sector = intersection.sector + 1; // from 0-based to 1-based enumeration
          int layer = intersection.layer + 1; // from 0-based to 1-based enumeration
          const bklm::Module* m = bklm::GeometryPar::instance()->findModule(section, sector, layer); // uses 1-based enumeration
          if (m) {
            const CLHEP::Hep3Vector localPosition = m->globalToLocal(intersection.position); // uses and returns position in cm
            int zStrip = m->getZStripNumber(localPosition);
            int phiStrip = m->getPhiStripNumber(localPosition);
            if (zStrip >= 0 && phiStrip >= 0) {
              uint16_t channel1, channel2;
              channel1 = m_klmElementNumbers->channelNumberBKLM(
                           section, sector, layer,
                           BKLMElementNumbers::c_ZPlane, zStrip);
              channel2 = m_klmElementNumbers->channelNumberBKLM(
                           section, sector, layer,
                           BKLMElementNumbers::c_PhiPlane, phiStrip);
              enum KLMChannelStatus::ChannelStatus status1, status2;
              status1 = m_klmChannelStatus->getChannelStatus(channel1);
              status2 = m_klmChannelStatus->getChannelStatus(channel2);
              if (status1 == KLMChannelStatus::c_Unknown ||
                  status2 == KLMChannelStatus::c_Unknown)
                B2ERROR("No KLM channel status data."
                        << LogVar("Section", section)
                        << LogVar("Sector", sector)
                        << LogVar("Layer", layer)
                        << LogVar("Z strip", zStrip)
                        << LogVar("Phi strip", phiStrip));
              isDead = (status1 == KLMChannelStatus::c_Dead ||
                        status2 == KLMChannelStatus::c_Dead);
            }
          }
          if (!isDead) {
            extState.extLayerPattern |= (0x00000001 << intersection.layer); // valid extrapolation-crossing of the layer but no matching hit
            float layerBarrelEfficiency = 1.;
            for (int plane = 0; plane <= BKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
              layerBarrelEfficiency *= m_klmStripEfficiency->getBarrelEfficiency(
                                         intersection.isForward ? BKLMElementNumbers::c_ForwardSection : BKLMElementNumbers::c_BackwardSection,
                                         intersection.sector + 1, intersection.layer + 1, plane, 1);
            }
            klmMuidLikelihood->setExtBKLMEfficiencyValue(intersection.layer, layerBarrelEfficiency);
          } else {
            klmMuidLikelihood->setExtBKLMEfficiencyValue(intersection.layer, 0);
          }
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
        float layerEndcapEfficiency = 1.;
        for (int plane = 1; plane <= EKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
          layerEndcapEfficiency *= m_klmStripEfficiency->getEndcapEfficiency(
                                     intersection.isForward ? EKLMElementNumbers::c_ForwardSection : EKLMElementNumbers::c_BackwardSection,
                                     intersection.sector + 1, intersection.layer + 1, plane, 1);
        }
        klmMuidLikelihood->setExtEKLMEfficiencyValue(intersection.layer, layerEndcapEfficiency);
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
        bool isDead = true;
        int result, strip1, strip2;
        result = m_eklmTransformData->getStripsByIntersection(
                   intersection.position, &strip1, &strip2);
        if (result == 0) {
          uint16_t channel1, channel2;
          channel1 = m_klmElementNumbers->channelNumberEKLM(strip1);
          channel2 = m_klmElementNumbers->channelNumberEKLM(strip2);
          enum KLMChannelStatus::ChannelStatus status1, status2;
          status1 = m_klmChannelStatus->getChannelStatus(channel1);
          status2 = m_klmChannelStatus->getChannelStatus(channel2);
          if (status1 == KLMChannelStatus::c_Unknown ||
              status2 == KLMChannelStatus::c_Unknown)
            B2ERROR("Incomplete KLM channel status data.");
          isDead = (status1 == KLMChannelStatus::c_Dead ||
                    status2 == KLMChannelStatus::c_Dead);
        }
        if (!isDead) {
          extState.extLayerPattern |= (0x00008000 << intersection.layer); // valid extrapolation-crossing of the layer but no matching hit
          float layerEndcapEfficiency = 1.;
          for (int plane = 1; plane <= EKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
            layerEndcapEfficiency *= m_klmStripEfficiency->getEndcapEfficiency(
                                       intersection.isForward ? EKLMElementNumbers::c_ForwardSection : EKLMElementNumbers::c_BackwardSection,
                                       intersection.sector + 1, intersection.layer + 1, plane, 1);
          }
          klmMuidLikelihood->setExtEKLMEfficiencyValue(intersection.layer, layerEndcapEfficiency);
        } else {
          klmMuidLikelihood->setExtEKLMEfficiencyValue(intersection.layer, 0);
        }
        if (extState.lastEndcapExtLayer < intersection.layer) {
          extState.lastEndcapExtLayer = intersection.layer;
        }
      }
    }
  }

  // Create a new MuidHit and RelationEntry between it and the track.
  // Adjust geant4e's position, momentum and covariance based on matching hit and tell caller to update the geant4e state.
  if (intersection.chi2 >= 0.0) {
    TVector3 tpos(intersection.position.x(), intersection.position.y(), intersection.position.z());
    TVector3 tposAtHitPlane(intersection.positionAtHitPlane.x(),
                            intersection.positionAtHitPlane.y(),
                            intersection.positionAtHitPlane.z());
    KLMMuidHit* klmMuidHit = m_klmMuidHits.appendNew(extState.pdgCode, intersection.inBarrel, intersection.isForward,
                                                     intersection.sector,
                                                     intersection.layer, tpos,
                                                     tposAtHitPlane, extState.tof, intersection.time, intersection.chi2);
    if (extState.track != nullptr) { extState.track->addRelationTo(klmMuidHit); }
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
  if (std::fabs(intersection.position.z() - m_OffsetZ) > m_BarrelHalfLength)
    return false;
  double phi = intersection.position.phi();
  if (phi < 0.0)
    phi += TWOPI;
  if (phi > TWOPI - PI_8)
    phi -= TWOPI;
  int sector = (int)((phi + PI_8) / M_PI_4);
  int section = intersection.position.z() > m_OffsetZ ?
                BKLMElementNumbers::c_ForwardSection :
                BKLMElementNumbers::c_BackwardSection;
  double oldR = oldPosition * m_BarrelSectorPerp[sector];
  double newR = intersection.position * m_BarrelSectorPerp[sector];
  for (int layer = extState.firstBarrelLayer; layer <= m_OutermostActiveBarrelLayer; ++layer) {
    if (newR <  m_BarrelModuleMiddleRadius[section][sector][layer]) break;
    if (oldR <= m_BarrelModuleMiddleRadius[section][sector][layer]) {
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
  if (oldPosition.perp() > m_EndcapMaxR)
    return false;
  if (intersection.position.perp() < m_EndcapMinR)
    return false;
  double oldZ = std::fabs(oldPosition.z() - m_OffsetZ);
  double newZ = std::fabs(intersection.position.z() - m_OffsetZ);
  bool isForward = intersection.position.z() > m_OffsetZ;
  int outermostLayer = isForward ? m_OutermostActiveForwardEndcapLayer
                       : m_OutermostActiveBackwardEndcapLayer;
  for (int layer = extState.firstEndcapLayer; layer <= outermostLayer; ++layer) {
    if (newZ <  m_EndcapModuleMiddleZ[layer])
      break;
    if (oldZ <= m_EndcapModuleMiddleZ[layer]) {
      extState.firstEndcapLayer = layer + 1; // ratchet outward for next call's loop starting value
      if (extState.firstEndcapLayer > outermostLayer)
        extState.escaped = true;
      intersection.inBarrel = false;
      intersection.isForward = isForward;
      intersection.layer = layer;
      intersection.sector = m_eklmTransformData->getSectorByPosition(
                              isForward ? 2 : 1, intersection.position) - 1;
      return true;
    }
  }
  return false;
}

bool TrackExtrapolateG4e::findMatchingBarrelHit(Intersection& intersection, const Track* track)

{
  G4ThreeVector extPos0(intersection.position);
  double diffBestMagSq = 1.0E60;
  int bestHit = -1;
  int matchingLayer = intersection.layer + 1;
  G4ThreeVector n(m_BarrelSectorPerp[intersection.sector]);
  for (int h = 0; h < m_bklmHit2ds.getEntries(); ++h) {
    BKLMHit2d* hit = m_bklmHit2ds[h];
    if (hit->getLayer() != matchingLayer)
      continue;
    if (hit->isOutOfTime())
      continue;
    if (std::fabs(hit->getTime() - m_MeanDt) > m_MaxDt)
      continue;
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
      if (std::fabs(dn) > 50.0)
        continue;
      int sector = hit->getSector() - 1;
      int dSector = abs(intersection.sector - sector);
      if ((dSector != +1) && (dSector != m_BarrelNSector - 1))
        continue;
      // Use the normal vector of the adjacent (hit's) sector
      G4ThreeVector nHit(m_BarrelSectorPerp[sector]);
      int section = intersection.isForward ?
                    BKLMElementNumbers::c_ForwardSection :
                    BKLMElementNumbers::c_BackwardSection;
      double dn2 = intersection.position * nHit - m_BarrelModuleMiddleRadius[section][sector][intersection.layer];
      dn = diff * nHit + dn2;
      if (std::fabs(dn) > 1.0)
        continue;
      // Project extrapolated track to the hit's plane in the adjacent sector
      G4ThreeVector extDir(intersection.momentum.unit());
      double extDirA = extDir * nHit;
      if (std::fabs(extDirA) < 1.0E-6)
        continue;
      G4ThreeVector projection = extDir * (dn2 / extDirA);
      if (projection.mag() > 15.0)
        continue;
      diff += projection - nHit * dn;
      if (diff.mag2() < diffBestMagSq) {
        diffBestMagSq = diff.mag2();
        bestHit = h;
        extPos0 = intersection.position - projection;
      }
    }
  }

  if (bestHit >= 0) {
    BKLMHit2d* hit = m_bklmHit2ds[bestHit];
    intersection.isForward = (hit->getSection() == 1);
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
      if (track != nullptr) {
        track->addRelationTo(hit);
        RecoTrack* recoTrack = track->getRelatedTo<RecoTrack>();
        if (m_addHitsToRecoTrack) {
          recoTrack->addBKLMHit(hit, recoTrack->getNumberOfTotalHits() + 1);
        }
      }
    }
  }
  return intersection.chi2 >= 0.0;

}

bool TrackExtrapolateG4e::findMatchingEndcapHit(Intersection& intersection, const Track* track)
{
  double diffBestMagSq = 1.0E60;
  int bestHit = -1;
  int matchingLayer = intersection.layer + 1;
  int matchingEndcap = (intersection.isForward ? 2 : 1);
  G4ThreeVector n(0.0, 0.0, (intersection.isForward ? 1.0 : -1.0));
  for (int h = 0; h < m_eklmHit2ds.getEntries(); ++h) {
    EKLMHit2d* hit = m_eklmHit2ds[h];
    if (hit->getLayer() != matchingLayer)
      continue;
    if (hit->getSection() != matchingEndcap)
      continue;
    // DIVOT no such function for EKLM!
    // if (hit->isOutOfTime()) continue;
    if (std::fabs(hit->getTime() - m_MeanDt) > m_MaxDt)
      continue;
    G4ThreeVector diff(hit->getPositionX() - intersection.position.x(),
                       hit->getPositionY() - intersection.position.y(),
                       hit->getPositionZ() - intersection.position.z());
    double dn = diff * n; // in cm
    if (std::fabs(dn) > 2.0)
      continue;
    diff -= n * dn;
    if (diff.mag2() < diffBestMagSq) {
      diffBestMagSq = diff.mag2();
      bestHit = h;
    }
  }

  if (bestHit >= 0) {
    EKLMHit2d* hit = m_eklmHit2ds[bestHit];
    intersection.hit = bestHit;
    intersection.isForward = (hit->getSection() == 2);
    intersection.sector = hit->getSector() - 1;
    intersection.time = hit->getTime();
    double localVariance[2] = {m_EndcapScintVariance, m_EndcapScintVariance};
    G4ThreeVector hitPos(hit->getPositionX(), hit->getPositionY(), hit->getPositionZ());
    adjustIntersection(intersection, localVariance, hitPos, intersection.position);
    if (intersection.chi2 >= 0.0) {
      // DIVOT no such function for EKLM!
      // hit->isOnTrack(true);
      if (track != nullptr) {
        track->addRelationTo(hit);
        RecoTrack* recoTrack = track->getRelatedTo<RecoTrack>();
        if (m_addHitsToRecoTrack) {
          for (const EKLMAlignmentHit& alignmentHit : hit->getRelationsFrom<EKLMAlignmentHit>()) {
            recoTrack->addEKLMHit(&alignmentHit, recoTrack->getNumberOfTotalHits() + 1);
          }
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
  if (std::fabs(extDirA) < 1.0E-6)
    return;
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
  if (residual[0][0] * residual[0][0] > correction[0][0] * m_MaxDistSqInVariances)
    return;
  if (residual[1][0] * residual[1][0] > correction[1][1] * m_MaxDistSqInVariances)
    return;
  int fail = 0;
  correction.invert(fail);
  if (fail != 0)
    return;
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
  if (fail != 0)
    return;
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

void TrackExtrapolateG4e::finishTrack(const ExtState& extState, KLMMuidLikelihood* klmMuidLikelihood, bool isForward)
{
  /* Done with this track: compute KLM likelihoods and fill the relative dataobject. */
  int lastExtLayer = extState.lastBarrelExtLayer + extState.lastEndcapExtLayer + 1;
  unsigned int outcome = MuidElementNumbers::calculateExtrapolationOutcome(
                           isForward, extState.escaped, extState.lastBarrelExtLayer, extState.lastEndcapExtLayer);
  klmMuidLikelihood->setOutcome(outcome);
  klmMuidLikelihood->setIsForward(isForward);
  klmMuidLikelihood->setBarrelExtLayer(extState.lastBarrelExtLayer);
  klmMuidLikelihood->setEndcapExtLayer(extState.lastEndcapExtLayer);
  klmMuidLikelihood->setBarrelHitLayer(extState.lastBarrelHitLayer);
  klmMuidLikelihood->setEndcapHitLayer(extState.lastEndcapHitLayer);
  klmMuidLikelihood->setExtLayer(lastExtLayer);
  klmMuidLikelihood->setHitLayer(((extState.lastEndcapHitLayer == -1) ?
                                  extState.lastBarrelHitLayer :
                                  extState.lastBarrelExtLayer + extState.lastEndcapHitLayer + 1));
  klmMuidLikelihood->setChiSquared(extState.chi2);
  klmMuidLikelihood->setDegreesOfFreedom(extState.nPoint);
  klmMuidLikelihood->setExtLayerPattern(extState.extLayerPattern);
  klmMuidLikelihood->setHitLayerPattern(extState.hitLayerPattern);
  /* Do KLM likelihood calculation. */
  if (outcome != MuidElementNumbers::c_NotReached) { /* Extrapolation reached KLM sensitive volume. */
    double denom = 0.0;
    int charge = klmMuidLikelihood->getCharge();
    std::vector<int> signedPdgVector = MuidElementNumbers::getPDGVector(charge);
    std::map<int, double> mapPdgPDF;
    for (int pdg : signedPdgVector) {
      auto search = m_MuidBuilderMap.find(pdg);
      if (search == m_MuidBuilderMap.end())
        B2FATAL("Something went wrong: PDF for PDG code " << pdg << " not found!");
      double pdf = (search->second)->getPDF(klmMuidLikelihood);
      denom += pdf;
      mapPdgPDF.insert(std::pair<int, double>(std::abs(pdg), pdf));
    }
    if (denom < 1.0E-20)
      klmMuidLikelihood->setJunkPDFValue(true); /* Anomaly: should be very rare. */
    else {
      for (auto const& [pdg, pdf] : mapPdgPDF) {
        klmMuidLikelihood->setPDFValue(pdf, std::abs(pdg));
        if (pdf > 0.0)
          klmMuidLikelihood->setLogL(std::log(pdf), std::abs(pdg));
      }
    }
  }
}
