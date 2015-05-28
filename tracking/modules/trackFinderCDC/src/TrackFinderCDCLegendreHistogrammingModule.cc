#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreHistogrammingModule.h>
#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohitsProcesser.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreHistogramming)

void CDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  //create object which will add stereohits to tracks
  StereohitsProcesser stereohitsProcesser;

  B2DEBUG(100, "Processing new track; assigning stereohits.");

  for (CDCTrack& track : tracks) {
    //assign stereohits to the track
    stereohitsProcesser.makeHistogramming(track);
  }
}
