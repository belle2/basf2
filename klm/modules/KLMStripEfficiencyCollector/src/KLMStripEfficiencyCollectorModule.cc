/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMStripEfficiencyCollector/KLMStripEfficiencyCollectorModule.h>
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TH1F.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

using namespace Belle2;

REG_MODULE(KLMStripEfficiencyCollector)

KLMStripEfficiencyCollectorModule::KLMStripEfficiencyCollectorModule() :
  CalibrationCollectorModule(),
  m_GeometryBKLM(nullptr)
{
  setDescription("Module for EKLM strips efficiency (data collection).");
  addParam("MuonListName", m_MuonListName, "Muon list name.",
           std::string("mu+:all"));
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Maximal distance in the units of strip number from ExtHit to "
           "matching (B|E)KLMDigit.", double(8));
  addParam("MinimalMatchingDigits", m_MinimalMatchingDigits,
           "Minimal number of matching digits.", 0);
  addParam("MinimalMatchingDigitsOuterLayers",
           m_MinimalMatchingDigitsOuterLayers,
           "Minimal number of matching digits in outer layers.", 0);
  addParam("MinimalMomentumNoOuterLayers", m_MinimalMomentumNoOuterLayers,
           "Minimal momentum in case there are no hits in outer layers.", 0.0);
  addParam("RemoveUnusedMuons", m_RemoveUnusedMuons,
           "Whether to remove unused muons.", false);
  addParam("Debug", m_Debug, "Debug mode.", false);
  setPropertyFlags(c_ParallelProcessingCertified);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
  m_PlaneArrayIndex = &(KLMPlaneArrayIndex::Instance());
}

KLMStripEfficiencyCollectorModule::~KLMStripEfficiencyCollectorModule()
{
}

void KLMStripEfficiencyCollectorModule::prepare()
{
  m_EklmDigits.isRequired();
  m_BklmDigits.isRequired();
  m_tracks.isRequired();
  m_extHits.isRequired();
  m_MuonList.isRequired(m_MuonListName);
  int nPlanes = m_PlaneArrayIndex->getNPlanes();
  TH1F* matchedDigitsInPlane = new TH1F(
    "matchedDigitsInPlane", "Number of matching (B|E)KLMDigits",
    nPlanes, -0.5, double(nPlanes) - 0.5);
  TH1F* allExtHitsInPlane = new TH1F(
    "allExtHitsInPlane", "Number of ExtHits",
    nPlanes, -0.5, double(nPlanes) - 0.5);
  registerObject<TH1F>("matchedDigitsInPlane", matchedDigitsInPlane);
  registerObject<TH1F>("allExtHitsInPlane", allExtHitsInPlane);
  m_GeometryBKLM = bklm::GeometryPar::instance();
  if (m_Debug) {
    m_MatchingFile = new TFile("matching.root", "recreate");
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
        uint16_t channel = klmChannel.getKLMChannelNumber();
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
  for (unsigned int i = 0; i < nMuons; ++i) {
    const Particle* muon = m_MuonList->getParticle(i);
    bool trackUsed = collectDataTrack(muon);
    if (m_RemoveUnusedMuons && !trackUsed)
      toRemove.push_back(muon->getArrayIndex());
  }
  if (m_RemoveUnusedMuons)
    m_MuonList->removeParticles(toRemove);
}

void KLMStripEfficiencyCollectorModule::addHit(
  std::map<uint16_t, struct HitData>& hitMap,
  uint16_t planeGlobal, struct HitData* hitData)
{
  std::map<uint16_t, struct HitData>::iterator it;
  it = hitMap.find(planeGlobal);
  /*
   * There may be more than one such hit e.g. if track crosses the edge
   * of the strips or WLS fiber groove. Select only one hit per plane.
   */
  if (it == hitMap.end()) {
    hitMap.insert(std::pair<uint16_t, struct HitData>(
                    planeGlobal, *hitData));
  }
}

void KLMStripEfficiencyCollectorModule::findMatchingDigit(
  struct HitData* hitData)
{
  if (hitData->subdetector == KLMElementNumbers::c_EKLM) {
    for (const EKLMDigit& digit : m_EklmDigits) {
      if (!(digit.getSection() == hitData->section &&
            digit.getLayer() == hitData->layer &&
            digit.getSector() == hitData->sector &&
            digit.getPlane() == hitData->plane))
        continue;
      if (fabs(digit.getStrip() - hitData->strip) < m_AllowedDistance1D) {
        hitData->eklmDigit = &digit;
        return;
      }
    }
  } else {
    for (const BKLMDigit& digit : m_BklmDigits) {
      if (!(digit.getSection() == hitData->section &&
            digit.getLayer() == hitData->layer &&
            digit.getSector() == hitData->sector &&
            digit.getPlane() == hitData->plane))
        continue;
      if (fabs(digit.getStrip() - hitData->strip) < m_AllowedDistance1D) {
        hitData->bklmDigit = &digit;
        return;
      }
    }
  }
}

bool KLMStripEfficiencyCollectorModule::collectDataTrack(const Particle* muon)
{
  const int nExtrapolationLayers =
    KLMElementNumbers::getMaximalExtrapolationLayer();
  const Track* track = muon->getTrack();
  TH1F* matchedDigitsInPlane = getObjectPtr<TH1F>("matchedDigitsInPlane");
  TH1F* allExtHitsInPlane = getObjectPtr<TH1F>("allExtHitsInPlane");
  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  std::map<uint16_t, struct HitData> selectedHits;
  std::map<uint16_t, struct HitData>::iterator it;
  uint16_t channel;
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
    uint16_t planeGlobal = 0;
    hitData.hit = &hit;
    hitData.eklmDigit = nullptr;
    hitData.bklmDigit = nullptr;
    if (hit.getDetectorID() == Const::EDetector::EKLM) {
      int stripGlobal = hit.getCopyID();
      hitData.subdetector = KLMElementNumbers::c_EKLM;
      m_ElementNumbersEKLM->stripNumberToElementNumbers(
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
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr) {
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
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr) {
      matchedDigitsInPlane->Fill(m_PlaneArrayIndex->getIndex(it->first));
      if (m_Debug) {
        std::memcpy(&m_MatchingHitData, &(it->second), sizeof(struct HitData));
        if (it->second.eklmDigit != nullptr)
          m_MatchedStrip = it->second.eklmDigit->getStrip();
        else
          m_MatchedStrip = it->second.bklmDigit->getStrip();
        m_MatchingTree->Fill();
      }
    }
  }
  return true;
}
