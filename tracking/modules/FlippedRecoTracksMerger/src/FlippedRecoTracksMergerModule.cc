/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/FlippedRecoTracksMerger/FlippedRecoTracksMergerModule.h>

using namespace Belle2;

REG_MODULE(FlippedRecoTracksMerger);

FlippedRecoTracksMergerModule::FlippedRecoTracksMergerModule() :
  Module()
{
  setDescription("Copies RecoTracks without their fit information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("inputStoreArrayNameFlipped", m_inputStoreArrayNameFlipped,
           "Name of the input StoreArray for flipped tracks");
  addParam("MVA2nd_cut", m_2nd_mva_cut, "the cut for 2nd flip mva",  m_2nd_mva_cut);
}

void FlippedRecoTracksMergerModule::initialize()
{
  //m_inputRecoTracks.isRequired(m_inputStoreArrayName);
}

void FlippedRecoTracksMergerModule::event()
{

  Belle2::StoreArray<RecoTrack> m_inputRecoTracks(m_inputStoreArrayName);
  Belle2::StoreArray<RecoTrack> m_inputRecoTracksFlipped(m_inputStoreArrayNameFlipped);
  Belle2::StoreArray<TrackFitResult> TrackFitResultsArray("TrackFitResults");

  // loop all the recoTracks
  for (int ireco = 0; ireco <  m_inputRecoTracks.getEntries(); ireco++) {

    RecoTrack* recoTrack = m_inputRecoTracks[ireco];
    // check if the recoTracks was fitted successfully
    if (not recoTrack->wasFitSuccessful()) {
      continue;
    }
    // get the related Belle2::Tracks
    Track* b2track = recoTrack->getRelatedFrom<Belle2::Track>();

    if (b2track) {

      // if the 2ndMVA was assigned, aka: passed the 1st MVA
      if (recoTrack->get2ndFlipQualityIndicator() != -999) {

        // get the related RecoTrack_flipped
        RecoTrack* RecoTrack_flipped =  recoTrack->getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

        if (RecoTrack_flipped) {

          // get the Tracks_flipped
          Track* b2trackFlipped = RecoTrack_flipped->getRelatedFrom<Belle2::Track>("Tracks_flipped");
          if (b2trackFlipped) {
            std::vector<Track::ChargedStableTrackFitResultPair> fitResultsAfter = b2trackFlipped->getTrackFitResults("TrackFitResults_flipped");
            std::vector<Track::ChargedStableTrackFitResultPair> fitResultsBefore = b2track->getTrackFitResults();

            // pass the 2nd MVA cuts
            if (recoTrack->get2ndFlipQualityIndicator() > m_2nd_mva_cut) {
              // loop over the original fitResults
              for (long unsigned int index = 0; index < fitResultsBefore.size() ; index++) {
                // update the fitResults
                if (index < fitResultsAfter.size()) {
                  auto fitResultAfter  = fitResultsAfter[index].second;

                  fitResultsBefore[index].second->updateTrackFitResult(*fitResultAfter);
                } else {
                  fitResultsBefore[index].second->maskThisFitResult();
                }
              }


              const auto& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromLastHit();

              TVector3 currentPosition = measuredStateOnPlane.getPos();
              TVector3 currentMomentum = measuredStateOnPlane.getMom();
              double currentCharge = measuredStateOnPlane.getCharge();

              // revert the charge and momentum
              recoTrack->setChargeSeed(-currentCharge);
              recoTrack->setPositionAndMomentum(currentPosition,  -currentMomentum);

              // Reverse the SortingParameters
              auto RecoHitInfos = recoTrack->getRecoHitInformations();
              for (auto RecoHitInfo : RecoHitInfos) {
                RecoHitInfo->setSortingParameter(std::numeric_limits<unsigned int>::max() - RecoHitInfo->getSortingParameter());
              }

            }
          }
        }
      }
    }
  }
}
