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
  if (evtT0) {
    auto svdHypos = evtT0->getTemporaryEventT0s(Const::EDetector::SVD);
    if (svdHypos.size() > 0) { // if SVD eventT0 exists then loop over tracks, don't otherwise and leave their averageTime set at NaN
      // get the latest SVD hypothesis information, this is also the most accurate t0 value the SVD can provide
      const auto svdBestT0 = svdHypos.back();
      for (int i = 0; i < tracks.getEntries(); i++) {
        // Access Track
        const auto& Track = tracks.operator[](i);
        // Access related recoTrack
        const auto& recoTrack = Track->getRelatedTo<RecoTrack>(m_recoTrackColName);
        // Get SVD hits
        std::vector SVDHitsList = recoTrack->getSVDHitList();
        if (SVDHitsList.size() == 0) {
          Track->setTrackTime(std::numeric_limits<float>::quiet_NaN());
        } else {
          float averageTime = 0;
          for (auto const& hit : SVDHitsList) { // Compute average of ClsTime
            averageTime = averageTime + hit->getClsTime();
          }
          averageTime = averageTime / (SVDHitsList.size());
          Track->setTrackTime(averageTime - svdBestT0.eventT0);
        }
      }
    }
  }

}


