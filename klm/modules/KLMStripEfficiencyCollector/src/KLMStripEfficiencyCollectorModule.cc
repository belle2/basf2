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

/* ROOT headers. */
#include <TH1F.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

using namespace Belle2;

REG_MODULE(KLMStripEfficiencyCollector)

KLMStripEfficiencyCollectorModule::KLMStripEfficiencyCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for EKLM strips efficiency (data collection).");
  addParam("MuonListName", m_MuonListName,
           "Muon list name. If empty, use tracks.", std::string("mu+:all"));
  addParam("MinimalMatchingDigits", m_MinimalMatchingDigits,
           "Minimal number of matching digits.", 0);
  addParam("RemoveUnusedMuons", m_RemoveUnusedMuons,
           "Whether to remove unused muons.", false);
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Maximal distance in the units of strip number from ExtHit to "
           "matching (B|E)KLMDigit.", double(8));
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
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_extHits.isRequired();
  if (m_MuonListName != "")
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
}

void KLMStripEfficiencyCollectorModule::startRun()
{
  m_GeometryBKLM = bklm::GeometryPar::instance();
}

void KLMStripEfficiencyCollectorModule::closeRun()
{
}

void KLMStripEfficiencyCollectorModule::collect()
{
  if (m_MuonListName != "") {
    std::vector<unsigned int> toRemove;
    unsigned int nMuons = m_MuonList->getListSize();
    for (unsigned int i = 0; i < nMuons; ++i) {
      const Particle* particle = m_MuonList->getParticle(i);
      const Track* track = particle->getTrack();
      bool trackUsed = collectDataTrack(track);
      if (m_RemoveUnusedMuons && !trackUsed)
        toRemove.push_back(particle->getArrayIndex());
    }
    if (m_RemoveUnusedMuons)
      m_MuonList->removeParticles(toRemove);
  } else {
    for (const Track& track : m_tracks)
      collectDataTrack(&track);
  }
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

bool KLMStripEfficiencyCollectorModule::collectDataTrack(const Track* track)
{
  TH1F* matchedDigitsInPlane = getObjectPtr<TH1F>("matchedDigitsInPlane");
  TH1F* allExtHitsInPlane = getObjectPtr<TH1F>("allExtHitsInPlane");
  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  std::map<uint16_t, struct HitData> selectedHits;
  std::map<uint16_t, struct HitData>::iterator it;
  uint16_t channel;
  enum KLMChannelStatus::ChannelStatus status;
  struct HitData hitData;
  TVector3 extHitPosition;
  CLHEP::Hep3Vector extHitPositionCLHEP, localPosition;
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
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    findMatchingDigit(&(it->second));
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr)
      nDigits++;
  }
  if (nDigits < m_MinimalMatchingDigits)
    return false;
  /* Write efficiency histograms */
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    int matchingDigits = nDigits;
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr)
      matchingDigits--;
    if (matchingDigits < m_MinimalMatchingDigits)
      continue;
    allExtHitsInPlane->Fill(m_PlaneArrayIndex->getIndex(it->first));
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr)
      matchedDigitsInPlane->Fill(m_PlaneArrayIndex->getIndex(it->first));
  }
  return true;
}
