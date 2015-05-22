#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohitsProcesser.h>

#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreHistogrammingModule.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>

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
    B2DEBUG(100, "Processing new track; assigning stereohits.");

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

    // Get the trajectory2D from the old track (that should not have changed in this module)
    const CDCTrajectory2D& trajectory2D = oldCDCTrack->getStartTrajectory3D().getTrajectory2D();


    // Make a fit to determine the trajectory in sz direction
    CDCObservations2D szObervations;
    CDCTrajectorySZ trajectorySZ = CDCTrajectorySZ::basicAssumption();

    const std::vector<TrackHit*>& trackHitVector = trackCand->getTrackHits();
    for (const TrackHit* trackHit : trackHitVector) {
      // TODO: Can we determine the plane?
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(
                                        trackHit->getOriginalCDCHit(), 0);
      rlWireHit->getWireHit().getAutomatonCell().setTakenFlag();
      if (not trackHit->getIsAxial()) {
        const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit,
                                           trajectory2D);
        oldCDCTrack->push_back(std::move(cdcRecoHit3D));
        szObervations.append(cdcRecoHit3D.getPerpS(), cdcRecoHit3D.getRecoZ());
      }
    }

    if (szObervations.size() > 3) {
      const CDCSZFitter& szFitter = CDCSZFitter::getFitter();
      szFitter.update(trajectorySZ, szObervations);
    }
    CDCTrajectory3D newTrajectory3D(trajectory2D, trajectorySZ);

    // Set the starting point to the first hit
    Vector3D startingPosition = oldCDCTrack->front().getRecoPos3D();
    double sStartingPosition = newTrajectory3D.calcPerpS(startingPosition);
    double zStartingPosition = newTrajectory3D.getTrajectorySZ().mapSToZ(sStartingPosition);
    newTrajectory3D.setLocalOrigin(Vector3D(startingPosition.xy(), zStartingPosition));
    oldCDCTrack->setStartTrajectory3D(newTrajectory3D);


    oldCDCTrack->sort();
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
