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
#include <klm/eklm/modules/EKLMTrackMatchCollector/EKLMTrackMatchCollectorModule.h>

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

REG_MODULE(EKLMTrackMatchCollector)

EKLMTrackMatchCollectorModule::EKLMTrackMatchCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for EKLM strips efficiency (data collection).");
  addParam("MuonListName", m_MuonListName,
           "Muon list name. If empty, use tracks.", std::string("mu+:all"));
  addParam("StandaloneTrackSelection", m_StandaloneTrackSelection,
           "Whether to use standalone track selection."
           " Always turn this off for cosmic data.", true);
  addParam("MinimalMatchingDigits", m_MinimalMatchingDigits,
           "Minimal number of matching digits.", 0);
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Max distance in strips number to 1D hit from extHit to be still matched (default 8 strips)", double(8));
  setPropertyFlags(c_ParallelProcessingCertified);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_GeoDat = nullptr;
}

EKLMTrackMatchCollectorModule::~EKLMTrackMatchCollectorModule()
{
}

void EKLMTrackMatchCollectorModule::prepare()
{

  m_digits.isRequired();
  m_hit2ds.isRequired();
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_extHits.isRequired();
  m_recoTracks.registerRelationTo(m_hit2ds);
  if (m_MuonListName != "")
    m_MuonList.isRequired(m_MuonListName);
  m_GeoDat = &(EKLM::GeometryData::Instance());

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

void EKLMTrackMatchCollectorModule::trackCheck(
  bool trackSelected[EKLMElementNumbers::getMaximalSectionNumber()],
  int requiredHits) const
{
  int sectionHits[EKLMElementNumbers::getMaximalSectionNumber()];
  for (int i = 0; i < EKLMElementNumbers::getMaximalSectionNumber(); ++i) {
    trackSelected[i] = false;
    sectionHits[i] = false;
  }
  for (const EKLMHit2d& hit : m_hit2ds)
    sectionHits[hit.getSection() - 1]++;
  if (sectionHits[EKLMElementNumbers::c_BackwardSection - 1] > requiredHits)
    trackSelected[EKLMElementNumbers::c_ForwardSection - 1] = true;
  if (sectionHits[EKLMElementNumbers::c_ForwardSection - 1] > requiredHits)
    trackSelected[EKLMElementNumbers::c_BackwardSection - 1] = true;
}

const EKLMDigit* EKLMTrackMatchCollectorModule::findMatchingDigit(
  const struct HitData* hitData, double allowedDistance) const
{
  for (const EKLMDigit& digit : m_digits) {
    if (digit.getSection() == hitData->section &&
        digit.getLayer() == hitData->layer &&
        digit.getSector() == hitData->sector &&
        digit.getPlane() == hitData->plane &&
        (fabs(digit.getStrip() - hitData->strip) < allowedDistance))
      return &digit;
  }
  return nullptr;
}

double EKLMTrackMatchCollectorModule::getSumTrackEnergy(const StoreArray<Track>& selected_tracks) const
{
  double eneregy = 0;
  for (auto trk : selected_tracks) {
    const TrackFitResult* fitResult = trk.getTrackFitResultWithClosestMass(Const::muon);
    eneregy += fitResult->getEnergy();
  }
  return eneregy;
}

void EKLMTrackMatchCollectorModule::collect()
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

void EKLMTrackMatchCollectorModule::collectDataTrack(const Track* track)
{
  bool trackSelected[EKLMElementNumbers::getMaximalSectionNumber()] =
  {true, true};
  if (m_StandaloneTrackSelection)
    trackCheck(trackSelected, 5);

  TH1F* MatchedDigitsInPlane;
  MatchedDigitsInPlane = getObjectPtr<TH1F>("MatchedDigitsInPlane");
  TH1F* AllExtHitsInPlane;
  AllExtHitsInPlane = getObjectPtr<TH1F>("AllExtHitsInPlane");

  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  std::map<int, struct HitData> selectedHits;
  std::map<int, struct HitData>::iterator it;
  struct HitData hitData;
  for (const ExtHit& hit : extHits) {
    if (hit.getDetectorID() != Const::EDetector::EKLM)
      continue;
    /* Choose hits that enter the sensitive volume. */
    if (hit.getStatus() != EXT_ENTER)
      continue;
    /*
     * There may be more than one such hit e.g. if track crosses the edge
     * of the strips or WLS fiber groove. Select only one hit per plane.
     */
    int stripGlobal = hit.getCopyID();
    m_ElementNumbers->stripNumberToElementNumbers(
      stripGlobal, &hitData.section, &hitData.layer, &hitData.sector,
      &hitData.plane, &hitData.strip);
    hitData.hit = &hit;
    hitData.digit = nullptr;
    int planeGlobal = m_ElementNumbers->planeNumber(
                        hitData.section, hitData.layer, hitData.sector, hitData.plane);
    it = selectedHits.find(planeGlobal);
    if (it == selectedHits.end())
      selectedHits.insert(std::pair<int, struct HitData>(planeGlobal, hitData));
  }
  /* Find matching digits. */
  int nDigits = 0;
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    it->second.digit = findMatchingDigit(&(it->second), m_AllowedDistance1D);
    if (it->second.digit != nullptr)
      nDigits++;
  }
  /* Write efficiency histograms */
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    if (!trackSelected[it->second.section - 1])
      continue;
    int matchingDigits = nDigits;
    if (it->second.digit != nullptr)
      matchingDigits--;
    if (matchingDigits < m_MinimalMatchingDigits)
      continue;
    AllExtHitsInPlane->Fill(it->first);
    if (it->second.digit != nullptr)
      MatchedDigitsInPlane->Fill(it->first);
  }
}

void EKLMTrackMatchCollectorModule::closeRun()
{
}
