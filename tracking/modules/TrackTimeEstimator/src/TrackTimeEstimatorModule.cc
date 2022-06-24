/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/TrackTimeEstimator/TrackTimeEstimatorModule.h>
#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;
REG_MODULE(TrackTimeEstimator);

TrackTimeEstimatorModule::TrackTimeEstimatorModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription(
    R"DOC(Computes the track time, defined as the difference between the average of SVD clusters time and the SVDEvent T0)DOC");
}

void TrackTimeEstimatorModule::initialize()
{
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_evtT0.isRequired();

}

void TrackTimeEstimatorModule::event()
{
  if (m_evtT0.isValid()) {
    auto svdHypos = m_evtT0->getTemporaryEventT0s(Const::EDetector::SVD);
    if (svdHypos.size() > 0) { // if SVD eventT0 exists then loop over tracks, don't otherwise and leave their averageTime set at NaN
      // get the latest SVD hypothesis information, this is also the most accurate t0 value the SVD can provide (see SVDEventT0 doc)
      const auto svdBestT0 = svdHypos.back();
      //for (int i = 0; i < m_tracks.getEntries(); i++) {
      for (auto& track : m_tracks) {
        // Access Track
        // const auto& track = m_tracks.operator[](i);
        // Access related recoTrack
        const auto& recoTrack = track.getRelatedTo<RecoTrack>();
        // Get SVD hits
        const std::vector SVDHitsList = recoTrack->getSVDHitList();
        if (SVDHitsList.size() > 0) {
          float averageTime = 0;
          for (auto const& hit : SVDHitsList) { // Compute average of ClsTime
            averageTime += hit->getClsTime();
          }
          averageTime = averageTime / (SVDHitsList.size());
          track.setTrackTime(averageTime - svdBestT0.eventT0);
        }
      }
    }
  }

}


