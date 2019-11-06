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

/* KLM headers. */
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>

/* ROOT headers. */
#include <TH1F.h>

/* CLHEP headers. */
#include <CLHEP/Geometry/Point3D.h>

/* C++ headers. */
#include <map>
#include <string>
#include <tuple>

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
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Max distance in strips number to 1D hit from extHit to be still matched (default 8 strips)", double(8));
  setPropertyFlags(c_ParallelProcessingCertified);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
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

  TH1F* MatchedDigitsInPlane =
    new TH1F("Matched Digits in planeNumber", "",
             EKLMElementNumbers::getMaximalPlaneGlobalNumber(),
             0.5, EKLMElementNumbers::getMaximalPlaneGlobalNumber() + 0.5);
  TH1F* AllExtHitsInPlane = new TH1F(
    "All ExtHits in planeNumber", "",
    EKLMElementNumbers::getMaximalPlaneGlobalNumber(),
    0.5, EKLMElementNumbers::getMaximalPlaneGlobalNumber() + 0.5);

  registerObject<TH1F>("MatchedDigitsInPlane", MatchedDigitsInPlane);
  registerObject<TH1F>("AllExtHitsInPlane", AllExtHitsInPlane);
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
    unsigned int nMuons = m_MuonList->getListSize();
    for (unsigned int i = 0; i < nMuons; ++i) {
      const Particle* particle = m_MuonList->getParticle(i);
      const Track* track = particle->getTrack();
      collectDataTrack(track);
    }
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

void KLMStripEfficiencyCollectorModule::collectDataTrack(const Track* track)
{
  TH1F* MatchedDigitsInPlane;
  MatchedDigitsInPlane = getObjectPtr<TH1F>("MatchedDigitsInPlane");
  TH1F* AllExtHitsInPlane;
  AllExtHitsInPlane = getObjectPtr<TH1F>("AllExtHitsInPlane");

  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  std::map<uint16_t, struct HitData> selectedHits;
  std::map<uint16_t, struct HitData>::iterator it;
  struct HitData hitData;
  TVector3 extHitPosition;
  CLHEP::Hep3Vector extHitPositionCLHEP, localPosition;
  for (const ExtHit& hit : extHits) {
    if (hit.getDetectorID() != Const::EDetector::EKLM)
      continue;
    /* Choose hits that enter the sensitive volume. */
    if (hit.getStatus() != EXT_ENTER)
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
      planeGlobal = m_ElementNumbers->planeNumberEKLM(
                      hitData.section, hitData.sector, hitData.layer,
                      hitData.plane);
      addHit(selectedHits, planeGlobal, &hitData);
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
        planeGlobal = m_ElementNumbers->planeNumberBKLM(
                        hitData.section, hitData.sector, hitData.layer,
                        hitData.plane);
        addHit(selectedHits, planeGlobal, &hitData);
      } else {
        /* For RPCs, the sensitive volume corresponds to both readout planes. */
        extHitPosition = hit.getPosition();
        extHitPositionCLHEP.setX(extHitPosition.X() / CLHEP::cm);
        extHitPositionCLHEP.setY(extHitPosition.Y() / CLHEP::cm);
        extHitPositionCLHEP.setZ(extHitPosition.Z() / CLHEP::cm);
        const bklm::Module* module =
          m_GeometryBKLM->findModule(hitData.section, hitData.sector,
                                     hitData.layer);
        localPosition = module->globalToLocal(extHitPositionCLHEP);
        hitData.plane = BKLMElementNumbers::c_ZPlane;
        hitData.strip = module->getZStrip(localPosition);
        hitData.localPosition = localPosition.z();
        addHit(selectedHits, planeGlobal, &hitData);
        hitData.plane = BKLMElementNumbers::c_PhiPlane;
        hitData.strip = module->getPhiStrip(localPosition);
        hitData.localPosition = localPosition.y();
        addHit(selectedHits, planeGlobal, &hitData);
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
  /* Write efficiency histograms */
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    int matchingDigits = nDigits;
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr)
      matchingDigits--;
    if (matchingDigits < m_MinimalMatchingDigits)
      continue;
    AllExtHitsInPlane->Fill(it->first);
    if (it->second.eklmDigit != nullptr || it->second.bklmDigit != nullptr)
      MatchedDigitsInPlane->Fill(it->first);
  }
}
