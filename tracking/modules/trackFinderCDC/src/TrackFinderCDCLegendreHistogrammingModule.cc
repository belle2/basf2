#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreHistogrammingModule.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreHistogramming)

void CDCLegendreHistogrammingModule::startNewEvent()
{
  B2INFO("**********   CDCLegendreHistogrammingModule  ************");
  B2DEBUG(100, "Initializing hits");
  m_cdcLegendreTrackProcessor.initializeHitListFromWireHitTopology();
}

void CDCLegendreHistogrammingModule::makeHistogramming()
{
  //create object which will add stereohits to tracks
  StereohitsProcesser stereohitsProcesser;
  for (TrackCandidate* cand : m_trackList) {
    B2INFO("Processing new track; assigning stereohits.");

    //assign stereohits to the track
    stereohitsProcesser.makeHistogramming(cand, m_cdcLegendreTrackProcessor.getStereoHitsList());
  }
}

void CDCLegendreHistogrammingModule::importTrackList(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  if (tracks.size() == 0) {
    B2WARNING("CDCTracking: Input tracks list is empty!");
    return;
  }

  for (CDCTrack& cdcTrack : tracks) {
    m_trackList.push_back(new TrackCandidateWithStereoHits(&cdcTrack));
  }

  //sort tracks by value of curvature
  m_trackList.sort([](const TrackCandidateWithStereoHits * a, const TrackCandidateWithStereoHits * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });


  B2DEBUG(100, "Number of track candidates: " << m_trackList.size());
}

void CDCLegendreHistogrammingModule::outputObjects()
{
  // TODO: We can not use the builtin method createCDCTrackCandidates of the trackProcessor here, because we have to deal with the old cdc track information separately!
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  for (TrackCandidateWithStereoHits* trackCand : m_trackList) {
    CDCTrack* oldCDCTrack = trackCand->getCDCTrackCand();
    TVector3 position = trackCand->getReferencePoint();
    TVector3 momentum = trackCand->getMomentumEstimation(true);
    CDCTrajectory3D newTrajectory3D(position, momentum,
                                    trackCand->getChargeSign());
    CDCTrajectory2D trajectory2D(Vector2D(position.x(), position.y()),
                                 Vector2D(momentum.x(), momentum.y()), trackCand->getChargeSign());
    oldCDCTrack->setStartTrajectory3D(newTrajectory3D);
    std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();
    unsigned int sortingParameter = 0;
    for (TrackHit* trackHit : trackHitVector) {
      //add only stereohits
      if (trackHit->getIsAxial())
        continue;

      // TODO: Can we determine the plane?
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(
                                        trackHit->getOriginalCDCHit(), 0);
      rlWireHit->getWireHit().getAutomatonCell().setTakenFlag();
      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit,
                                         trajectory2D);
      oldCDCTrack->push_back(std::move(cdcRecoHit3D));
      sortingParameter++;
    }
  }
  //clear pointers
  clear_pointer_vectors();
}

void CDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  startNewEvent();
  importTrackList(tracks);
  makeHistogramming();
  outputObjects();
}

void CDCLegendreHistogrammingModule::clear_pointer_vectors()
{
  m_cdcLegendreTrackProcessor.clearVectors();

  for (TrackCandidateWithStereoHits* track : m_trackList) {
    // DELETE THE HIST ALSO!!!!!
    delete track;
  }
  m_trackList.clear();
}
