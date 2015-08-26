#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCLegendreHistogrammingModule.h>
#include <tracking/trackFindingCDC/legendre/stereohits/StereohitsProcesser.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCLegendreHistogramming)

StereoHitFinderCDCLegendreHistogrammingModule::StereoHitFinderCDCLegendreHistogrammingModule() :
  TrackFinderCDCBaseModule(), m_stereohitsProcesser(nullptr)
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  addParam("debugOutput",
           m_param_debugOutput,
           "Flag to turn on debug output.",
           false);

  addParam("quadTreeLevel",
           m_param_quadTreeLevel,
           "The number of levels for the quad tree search.",
           static_cast<unsigned int>(6));

  addParam("minimumHitsInQuadtree",
           m_param_minimumHitsInQuadTree,
           "The minimum number of hits in a quad tree bin to be called as result.",
           static_cast<unsigned int>(5));

  addParam("useOldImplementation",
           m_param_useOldImplementation,
           "Whether to use the old implementation o the quad tree.",
           true);

  addParam("checkForB2BTracks", m_param_checkForB2BTracks,
           "Set to false to skip the check for back-2-back tracks (good for cosmics)",
           true);
}

/** Initialize the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::initialize()
{
  m_stereohitsProcesser = new TrackFindingCDC::StereohitsProcesser(m_param_quadTreeLevel, m_param_debugOutput,
      m_param_checkForB2BTracks);

  TrackFinderCDCBaseModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::terminate()
{
  delete m_stereohitsProcesser;

  TrackFinderCDCBaseModule::terminate();
}

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  //create object which will add stereohits to tracks


  if (m_param_useOldImplementation) {
    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser->makeHistogramming(track, m_param_minimumHitsInQuadTree);
      track.sortByPerpS();
    }
  } else {
    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser->makeHistogrammingWithNewQuadTree(track, m_param_minimumHitsInQuadTree);
      track.sortByPerpS();
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
