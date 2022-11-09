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
      for (auto& track : m_tracks) {
        // Access related recoTrack
        const auto& recoTrack = track.getRelatedTo<RecoTrack>();
        // if the outgoing arm exists, the track time is computed as the difference of the outgoing arm time and the SVD EventT0
        // otherwise, if the ingoing arm exists, the track time is computed as the difference of the ingoing arm time and the SVD EventT0
        float outgoingArmTime = recoTrack->getOutgoingArmTime();
        float ingoingArmTime = recoTrack->getIngoingArmTime();
        if (recoTrack->hasOutgoingArmTime()) {
          track.setTrackTime(outgoingArmTime - svdBestT0.eventT0);
        } else if (recoTrack->hasIngoingArmTime()) {
          track.setTrackTime(ingoingArmTime - svdBestT0.eventT0);
        }
      }
    }
  }
}
