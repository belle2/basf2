#pragma once

#include <fstream>

#include <boost/tuple/tuple.hpp>
#include <vector>

#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreTrackWithStereohits.h>
#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohitsProcesser.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>

#include <tracking/modules/trackFinderCDC/TrackFinderCDCBaseModule.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class TrackHit;
  }

  class CDCLegendreHistogrammingModule: public TrackFinderCDCBaseModule {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     */
    CDCLegendreHistogrammingModule() : TrackFinderCDCBaseModule() { }

  private:
    std::list<TrackFindingCDC::TrackCandidateWithStereoHits*>
    m_trackList; /**< List of track candidates. Mainly used for memory management! Later we want to use the builtin list in the trackProcessor.*/
    TrackFindingCDC::TrackProcessor m_cdcLegendreTrackProcessor; /** This object is used for memory handling*/

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override;

    /**
     * Release all the used pointers.
     */
    void clear_pointer_vectors();

    /**
     * Fill the used lists.
     */
    void startNewEvent();

    /**
     * Do the real stereo hit finding.
     */
    void makeHistogramming();

    /**
     * Go through all tracks and import them to our own track list.
     */
    void importTrackList(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

    /**
     * Update the input cdctracks and output them to the store array.
     */
    void outputObjects();
  };

}
