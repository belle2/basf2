#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCLegendreHistogrammingModule.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCLegendreHistogramming)

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  //create object which will add stereohits to tracks


  if (m_param_useOldImplementation) {
    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser->makeHistogramming(track, m_param_minimumHitsInQuadTree);
      track.sort();
    }
  } else {
    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser->makeHistogrammingWithNewQuadTree(track, m_param_minimumHitsInQuadTree);
      track.sort();
    }
  }

  // Fit the tracks
  const CDCSZFitter& fitter = CDCSZFitter::getFitter();

  for (CDCTrack& track : tracks) {
    CDCObservations2D szObservations;
    for (const CDCRecoHit3D& recoHit : track) {
      if (recoHit.getStereoType() != StereoType_c::Axial) {
        szObservations.append(recoHit.getPerpS(), recoHit.getRecoZ());
      }
    }

    if (szObservations.size() > 2) {
      CDCTrajectorySZ szTrajectory;
      fitter.update(szTrajectory, track);

      CDCTrajectory3D trajectory3D(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
      track.setStartTrajectory3D(trajectory3D);
    }
  }
}
