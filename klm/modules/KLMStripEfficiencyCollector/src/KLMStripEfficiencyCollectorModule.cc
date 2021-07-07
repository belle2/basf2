/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMStripEfficiencyCollector/KLMStripEfficiencyCollectorModule.h>
#include <klm/dataobjects/KLMChannelIndex.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

using namespace Belle2;

REG_MODULE(KLMStripEfficiencyCollector)

KLMStripEfficiencyCollectorModule::KLMStripEfficiencyCollectorModule() :
  CalibrationCollectorModule(),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance())),
  m_GeometryBKLM(nullptr),
  m_PlaneArrayIndex(&(KLMPlaneArrayIndex::Instance())),
  m_MatchingFile(nullptr),
  m_MatchingTree(nullptr),
  m_MatchingHitData( {0, 0, 0, 0, 0, 0, 0., nullptr, nullptr}),
                   m_MatchedStrip(0)
{
  setDescription("Module for KLM strip efficiency data collection.");
  addParam("MuonListName", m_MuonListName, "Muon list name.",
           std::string("mu+:all"));
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Maximal distance in the units of strip number from ExtHit to "
           "matching KLMDigit.", double(8));
  addParam("MinimalMatchingDigits", m_MinimalMatchingDigits,
           "Minimal number of matching digits.", 0);
  addParam("MinimalMatchingDigitsOuterLayers",
           m_MinimalMatchingDigitsOuterLayers,
           "Minimal number of matching digits in outer layers.", 0);
  addParam("MinimalMomentumNoOuterLayers", m_MinimalMomentumNoOuterLayers,
           "Minimal momentum in case there are no hits in outer layers.", 0.0);
  addParam("RemoveUnusedMuons", m_RemoveUnusedMuons,
           "Whether to remove unused muons.", false);
  addParam("IgnoreBackwardPropagation", m_IgnoreBackwardPropagation,
           "Whether to ignore ExtHits with backward propagation.", false);
  addParam("Debug", m_Debug, "Debug mode.", false);
  addParam("DebugFileName", m_MatchingFileName, "Debug file name.", std::string("matching.root"));
  setPropertyFlags(c_ParallelProcessingCertified);
}

KLMStripEfficiencyCollectorModule::~KLMStripEfficiencyCollectorModule()
{
}

void KLMStripEfficiencyCollectorModule::prepare()
{
  m_Digits.isRequired();
  m_tracks.isRequired();
  m_extHits.isRequired();
  m_MuonList.isRequired(m_MuonListName);
  int nPlanes = m_PlaneArrayIndex->getNElements();
  TH1F* matchedDigitsInPlane = new TH1F(
    "matchedDigitsInPlane", "Number of matching KLMDigits",
    nPlanes, -0.5, double(nPlanes) - 0.5);
  TH1F* allExtHitsInPlane = new TH1F(
    "allExtHitsInPlane", "Number of ExtHits",
    nPlanes, -0.5, double(nPlanes) - 0.5);
  registerObject<TH1F>("matchedDigitsInPlane", matchedDigitsInPlane);
  registerObject<TH1F>("allExtHitsInPlane", allExtHitsInPlane);
  m_GeometryBKLM = bklm::GeometryPar::instance();
  if (m_Debug) {
    m_MatchingFile = new TFile(m_MatchingFileName.c_str(), "recreate");
    m_MatchingTree = new TTree("t_matching", "");
    m_MatchingTree->Branch("subdetector", &m_MatchingHitData.subdetector,
                           "subdetector/I");
    m_MatchingTree->Branch("section", &m_MatchingHitData.section, "section/I");
    m_MatchingTree->Branch("sector", &m_MatchingHitData.sector, "sector/I");
    m_MatchingTree->Branch("layer", &m_MatchingHitData.layer, "layer/I");
    m_MatchingTree->Branch("plane", &m_MatchingHitData.plane, "plane/I");
    m_MatchingTree->Branch("strip", &m_MatchingHitData.strip, "strip/I");
    m_MatchingTree->Branch("matchedStrip", &m_MatchedStrip, "matchedStrip/I");
  }
}

void KLMStripEfficiencyCollectorModule::finish()
{
  if (m_Debug) {
    m_MatchingFile->cd();
    m_MatchingTree->Write();
    delete m_MatchingTree;
    delete m_MatchingFile;
  }
}

void KLMStripEfficiencyCollectorModule::startRun()
{
  if (!m_ChannelStatus.isValid())
    B2FATAL("KLM channel status data are not available.");
  int minimalActivePlanes = -1;
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    KLMChannelIndex klmNextSector(klmSector);
    ++klmNextSector;
    int activePlanes = 0;
    KLMChannelIndex klmPlane(klmSector);
    klmPlane.setIndexLevel(KLMChannelIndex::c_IndexLevelPlane);
    for (; klmPlane != klmNextSector; ++klmPlane) {
      KLMChannelIndex klmNextPlane(klmPlane);
      ++klmNextPlane;
      bool isActive = false;
      KLMChannelIndex klmChannel(klmPlane);
      klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
      for (; klmChannel != klmNextPlane; ++ klmChannel) {
        KLMChannelNumber channel = klmChannel.getKLMChannelNumber();
        enum KLMChannelStatus::ChannelStatus status =
          m_ChannelStatus->getChannelStatus(channel);
        if (status == KLMChannelStatus::c_Unknown)
          B2FATAL("Incomplete KLM channel status data.");
        if (status == KLMChannelStatus::c_Normal) {
          isActive = true;
          break;
        }
      }
      if (isActive)
        ++activePlanes;
    }
    /*
     * If a sector is completely off, it is not necessary to reduce
     * the minimal number of digits, because the efficiencies cannot be
     * measured for the entire sector anyway.
     */
    if (activePlanes == 0)
      continue;
    if (minimalActivePlanes < 0)
      minimalActivePlanes = activePlanes;
    else if (minimalActivePlanes < activePlanes)
      minimalActivePlanes = activePlanes;
  }
  if ((minimalActivePlanes >= 0) &&
      (minimalActivePlanes < m_MinimalMatchingDigits)) {
    B2WARNING("The minimal number of active planes (" << minimalActivePlanes <<
              ") is less than the minimal number of matching digits (" <<
              m_MinimalMatchingDigits << "). The minimal number of "
              "matching digits is reduced to make the calibration possible.");
    m_MinimalMatchingDigits = minimalActivePlanes;
  }
}

void KLMStripEfficiencyCollectorModule::closeRun()
{
}

void KLMStripEfficiencyCollectorModule::collect()
{
  std::vector<unsigned int> toRemove;
  unsigned int nMuons = m_MuonList->getListSize();
  /*
   * The getter functions create the object for particular experiment and run.
   * It is not guaranteed that collectDataTrack() is called if there are
   * no tracks (e.g. for too short or bad runs). Thus, they are called here
   * to guarantee the creation of histograms.
   */
  TH1F* matchedDigitsInPlane = getObjectPtr<TH1F>("matchedDigitsInPlane");
  TH1F* allExtHitsInPlane = getObjectPtr<TH1F>("allExtHitsInPlane");
  for (unsigned int i = 0; i < nMuons; ++i) {
    const Particle* muon = m_MuonList->getParticle(i);
    bool trackUsed = collectDataTrack(muon, matchedDigitsInPlane,
                                      allExtHitsInPlane);
    if (m_RemoveUnusedMuons && !trackUsed)
      toRemove.push_back(muon->getArrayIndex());
  }
  if (m_RemoveUnusedMuons)
    m_MuonList->removeParticles(toRemove);
}

void KLMStripEfficiencyCollectorModule::addHit(
  std::map<KLMPlaneNumber, struct HitData>& hitMap,
  KLMPlaneNumber planeGlobal, struct HitData* hitData)
{
  std::map<KLMPlaneNumber, struct HitData>::iterator it;
  it = hitMap.find(planeGlobal);
  /*
   * There may be more than one such hit e.g. if track crosses the edge
   * of the strips or WLS fiber groove. Select only one hit per plane.
   */
  if (it == hitMap.end()) {
    hitMap.insert(std::pair<KLMPlaneNumber, struct HitData>(
                    planeGlobal, *hitData));
  }
}

void KLMStripEfficiencyCollectorModule::findMatchingDigit(
  struct HitData* hitData)
{
  for (const KLMDigit& digit : m_Digits) {
    /*
     * TODO: multi-strip digits are ugnored for now.
     * It is necessary to take them into account.
     */
    if (digit.isMultiStrip())
      continue;
    if (!(digit.getSubdetector() == hitData->subdetector &&
          digit.getSection() == hitData->section &&
          digit.getLayer() == hitData->layer &&
          digit.getSector() == hitData->sector &&
          digit.getPlane() == hitData->plane))
      continue;
    if (fabs(digit.getStrip() - hitData->strip) < m_AllowedDistance1D) {
      hitData->digit = &digit;
      return;
    }
  }
}

bool KLMStripEfficiencyCollectorModule::collectDataTrack(
  const Particle* muon, TH1F* matchedDigitsInPlane, TH1F* allExtHitsInPlane)
{
  const int nExtrapolationLayers =
    KLMElementNumbers::getMaximalExtrapolationLayer();
  const Track* track = muon->getTrack();
  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  std::map<KLMPlaneNumber, struct HitData> selectedHits;
  std::map<KLMPlaneNumber, struct HitData>::iterator it;
  KLMChannelNumber channel;
  enum KLMChannelStatus::ChannelStatus status;
  struct HitData hitData, hitDataPrevious;
  TVector3 extHitPosition;
  CLHEP::Hep3Vector extHitPositionCLHEP, localPosition;
  int layer;
  int extHitLayer[nExtrapolationLayers] = {0};
  int digitLayer[nExtrapolationLayers] = {0};
  hitDataPrevious.subdetector = -1;
  for (const ExtHit& hit : extHits) {
    /*
     * Choose hits that exit the sensitive volume.
     * It is not possible to use entry hits because of a bug in Geant4:
     * the step does not always have a correct status (fGeomBoundary),
     * and, consequently, ExtHits are not created.
     */
    if (hit.getStatus() != EXT_EXIT)
      continue;
    /*
     * Ignore ExtHits with backward propagation. This affects cosmic events
     * only. The removal of hits with backward propagation is normally
     * not needed, however, it is added because of backward error propagation
     * bug in Geant4 10.6.
     */
    if (m_IgnoreBackwardPropagation) {
      if (hit.isBackwardPropagated())
        continue;
    }
    KLMPlaneNumber planeGlobal = 0;
    hitData.hit = &hit;
    hitData.digit = nullptr;
    if (hit.getDetectorID() == Const::EDetector::EKLM) {
      int stripGlobal = hit.getCopyID();
      hitData.subdetector = KLMElementNumbers::c_EKLM;
      m_eklmElementNumbers->stripNumberToElementNumbers(
        stripGlobal, &hitData.section, &hitData.layer, &hitData.sector,
        &hitData.plane, &hitData.strip);
      channel = m_ElementNumbers->channelNumberEKLM(
                  hitData.section, hitData.sector, hitData.layer,
                  hitData.plane, hitData.strip);
      status = m_ChannelStatus->getChannelStatus(channel);
      if (status == KLMChannelStatus::c_Unknown)
        B2FATAL("Incomplete KLM channel status data.");
      if (status == KLMChannelStatus::c_Normal) {
        layer = m_ElementNumbers->getExtrapolationLayer(
                  hitData.subdetector, hitData.layer);
        extHitLayer[layer - 1]++;
        planeGlobal = m_ElementNumbers->planeNumberEKLM(
                        hitData.section, hitData.sector, hitData.layer,
                        hitData.plane);
        addHit(selectedHits, planeGlobal, &hitData);
      }
    } else if (hit.getDetectorID() == Const::EDetector::BKLM) {
      int moduleNumber = hit.getCopyID();
      hitData.subdetector = KLMElementNumbers::c_BKLM;
      BKLMElementNumbers::moduleNumberToElementNumbers(
        moduleNumber, &hitData.section, &hitData.sector, &hitData.layer);
      if (hitData.layer < BKLMElementNumbers::c_FirstRPCLayer) {
        /*
         * For scintillators, the plane and strip numbers are recorded
         * in the copy number.
         */
        BKLMElementNumbers::channelNumberToElementNumbers(
          moduleNumber, &hitData.section, &hitData.sector, &hitData.layer,
          &hitData.plane, &hitData.strip);
        channel = m_ElementNumbers->channelNumberBKLM(
                    hitData.section, hitData.sector, hitData.layer,
                    hitData.plane, hitData.strip);
        status = m_ChannelStatus->getChannelStatus(channel);
        if (status == KLMChannelStatus::c_Unknown)
          B2FATAL("Incomplete KLM channel status data.");
        if (status == KLMChannelStatus::c_Normal) {
          layer = m_ElementNumbers->getExtrapolationLayer(
                    hitData.subdetector, hitData.layer);
          extHitLayer[layer - 1]++;
          planeGlobal = m_ElementNumbers->planeNumberBKLM(
                          hitData.section, hitData.sector, hitData.layer,
                          hitData.plane);
          addHit(selectedHits, planeGlobal, &hitData);
        }
      } else {
        /* For RPCs, the sensitive volume corresponds to both readout planes. */
        extHitPosition = hit.getPosition();
        extHitPositionCLHEP.setX(extHitPosition.X());
        extHitPositionCLHEP.setY(extHitPosition.Y());
        extHitPositionCLHEP.setZ(extHitPosition.Z());
        const bklm::Module* module =
          m_GeometryBKLM->findModule(hitData.section, hitData.sector,
                                     hitData.layer);
        localPosition = module->globalToLocal(extHitPositionCLHEP);
        hitData.plane = BKLMElementNumbers::c_ZPlane;
        hitData.strip = module->getZStrip(localPosition);
        /*
         * FIXME
         * There are 2 hits per module in RPCs, but the plane information is
         * not available in ExtHit. For now, 2 entries are created (one for
         * each plane) for the first hit, and the second one is removed.
         */
        if (hitData.subdetector == hitDataPrevious.subdetector &&
            hitData.section == hitDataPrevious.section &&
            hitData.sector == hitDataPrevious.sector &&
            hitData.layer == hitDataPrevious.layer)
          continue;
        std::memcpy(&hitDataPrevious, &hitData, sizeof(struct HitData));
        /* The returned strip may be out of the valid range. */
        if (BKLMElementNumbers::checkChannelNumber(
              hitData.section, hitData.sector, hitData.layer, hitData.plane,
              hitData.strip, false)) {
          channel = m_ElementNumbers->channelNumberBKLM(
                      hitData.section, hitData.sector, hitData.layer,
                      hitData.plane, hitData.strip);
          status = m_ChannelStatus->getChannelStatus(channel);
          if (status == KLMChannelStatus::c_Unknown)
            B2FATAL("Incomplete KLM channel status data.");
          if (status == KLMChannelStatus::c_Normal) {
            layer = m_ElementNumbers->getExtrapolationLayer(
                      hitData.subdetector, hitData.layer);
            extHitLayer[layer - 1]++;
            hitData.localPosition = localPosition.z();
            planeGlobal = m_ElementNumbers->planeNumberBKLM(
                            hitData.section, hitData.sector, hitData.layer,
                            hitData.plane);
            addHit(selectedHits, planeGlobal, &hitData);
          }
        }
        hitData.plane = BKLMElementNumbers::c_PhiPlane;
        hitData.strip = module->getPhiStrip(localPosition);
        /* The returned strip may be out of the valid range. */
        if (BKLMElementNumbers::checkChannelNumber(
              hitData.section, hitData.sector, hitData.layer, hitData.plane,
              hitData.strip, false)) {
          channel = m_ElementNumbers->channelNumberBKLM(
                      hitData.section, hitData.sector, hitData.layer,
                      hitData.plane, hitData.strip);
          status = m_ChannelStatus->getChannelStatus(channel);
          if (status == KLMChannelStatus::c_Unknown)
            B2FATAL("Incomplete KLM channel status data.");
          if (status == KLMChannelStatus::c_Normal) {
            layer = m_ElementNumbers->getExtrapolationLayer(
                      hitData.subdetector, hitData.layer);
            extHitLayer[layer - 1]++;
            hitData.localPosition = localPosition.y();
            planeGlobal = m_ElementNumbers->planeNumberBKLM(
                            hitData.section, hitData.sector, hitData.layer,
                            hitData.plane);
            addHit(selectedHits, planeGlobal, &hitData);
          }
        }
      }
    } else
      continue;
  }
  /* Find matching digits. */
  int nDigits = 0;
  std::map<int, int>::iterator it2;
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    findMatchingDigit(&(it->second));
    if (it->second.digit != nullptr) {
      nDigits++;
      layer = m_ElementNumbers->getExtrapolationLayer(
                it->second.subdetector, it->second.layer);
      digitLayer[layer - 1]++;
    }
  }
  if (nDigits < m_MinimalMatchingDigits)
    return false;
  /* Write efficiency histograms */
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    int matchingDigits = 0;
    int matchingDigitsOuterLayers = 0;
    int extHitsOuterLayers = 0;
    layer = m_ElementNumbers->getExtrapolationLayer(
              it->second.subdetector, it->second.layer) - 1;
    for (int i = 0; i < nExtrapolationLayers; ++i) {
      if (i != layer)
        matchingDigits += digitLayer[i];
      if (i > layer) {
        matchingDigitsOuterLayers += digitLayer[i];
        extHitsOuterLayers += extHitLayer[i];
      }
    }
    /* Check the number of matching digits in other layers. */
    if (matchingDigits < m_MinimalMatchingDigits)
      continue;
    /*
     * Check the number of matching digits in outer layers relatively to
     * this hit.
     */
    if (matchingDigitsOuterLayers < m_MinimalMatchingDigitsOuterLayers) {
      /**
       * It should be possible to have the required number of digits, thus,
       * the number of ExtHits needs to be sufficient. This requirement
       * does not reject the outer layers of the detector.
       */
      if (extHitsOuterLayers >= m_MinimalMatchingDigitsOuterLayers)
        continue;
      /*
       * If the number of ExtHits is insufficient, then check the momentum.
       * The muons with sufficiently large momentum have a very small
       * probability to get absorbed in the detector.
       */
      if (muon->getMomentum().Mag() < m_MinimalMomentumNoOuterLayers)
        continue;
    }
    allExtHitsInPlane->Fill(m_PlaneArrayIndex->getIndex(it->first));
    if (it->second.digit) {
      matchedDigitsInPlane->Fill(m_PlaneArrayIndex->getIndex(it->first));
      if (m_Debug) {
        std::memcpy(&m_MatchingHitData, &(it->second), sizeof(struct HitData));
        m_MatchedStrip = it->second.digit->getStrip();
        m_MatchingTree->Fill();
      }
    }
  }
  return true;
}
