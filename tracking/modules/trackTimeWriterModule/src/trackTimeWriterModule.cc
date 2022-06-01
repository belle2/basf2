/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackTimeWriterModule/trackTimeWriterModule.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>




/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
#include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;

REG_MODULE(trackTimeWriter)

trackTimeWriterModule::trackTimeWriterModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(Computes the track time, defined as the difference between the average of SVD clusters time and the SVDEvent T0)DOC");
  // input
  addParam("recoTrackColName", m_recoTrackColName, "Name of collection holding the RecoTracks (input).",
           m_recoTrackColName);
  addParam("trackColName", m_trackColName, "Name of collection holding the Tracks (input).", m_trackColName);

}

void trackTimeWriterModule::initialize()
{
  // Not entirely sure wether or not it's necessary
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  recoTracks.isRequired();
  StoreArray<Track> tracks(m_trackColName);
  tracks.isRequired();
}

void trackTimeWriterModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_recoTrackColName);
  StoreArray<Track> tracks(m_trackColName);
  // Accessing eventT0
  StoreObjPtr<EventT0> evtT0;
  double eventT0;
  if (!evtT0) {
    eventT0 = std::numeric_limits<double>::quiet_NaN();
  }
  if (evtT0->hasEventT0()) {
    eventT0 = evtT0->getEventT0();
    // if eventT0 exists then loop over tracks, don't otherwise
    for (int i = 0; i < tracks.getEntries(); i++) {
      const auto& Track = tracks.operator[](i);
      const auto& recoTrack = Track->getRelatedTo<RecoTrack>(m_recoTrackColName);
      std::vector SVDHitsList = recoTrack->getSVDHitList();
      if (SVDHitsList.size() == 0) {
        Track->setTrackTime(std::numeric_limits<float>::quiet_NaN());
      } else {
        float averageTime = 0;
        for (auto const& hit : SVDHitsList) {
          averageTime = averageTime + hit->getClsTime();
        }
        averageTime = averageTime / (SVDHitsList.size());
        Track->setTrackTime(averageTime - eventT0);
      }
    }
  } else { // actually not needed, either we go in previous if or we don't and then we don't need below var eventT0, just leaving the else for now
    // in case i need this condition later so i don't forget it
    eventT0 = std::numeric_limits<double>::quiet_NaN();
  }

}


