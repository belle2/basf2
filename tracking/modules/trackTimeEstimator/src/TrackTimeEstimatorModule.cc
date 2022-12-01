/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackTimeEstimator/TrackTimeEstimatorModule.h>
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

        // compute and set Track Time
        float outgoingArmTime = recoTrack->getOutgoingArmTime();
        float ingoingArmTime = recoTrack->getIngoingArmTime();

        // check if recoTrack has both ingoing and outgoing arms
        if (recoTrack->hasOutgoingArmTime() && recoTrack->hasIngoingArmTime()) {
          if (outgoingArmTime <= ingoingArmTime) {
            track.setTrackTime(outgoingArmTime - svdBestT0.eventT0);
          } else {
            track.setTrackTime(ingoingArmTime - svdBestT0.eventT0);
          }
        } else if (recoTrack->hasOutgoingArmTime() && !recoTrack->hasIngoingArmTime()) { // check if it has only outgoing arm
          track.setTrackTime(outgoingArmTime - svdBestT0.eventT0);
        } else if (!recoTrack->hasOutgoingArmTime() && recoTrack->hasIngoingArmTime()) { // check if it has only ingoing arm
          track.setTrackTime(ingoingArmTime - svdBestT0.eventT0);
        }
      }
    }
  }
}
