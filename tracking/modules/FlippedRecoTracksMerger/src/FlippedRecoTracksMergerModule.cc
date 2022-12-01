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
  setDescription("This module will check the 2 flipping QIs of one RecoTrack and update the original RecoTrack, Track and TrackFitResults if the flipped one should be taken (according to the 2 flipping QIs)");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("inputStoreArrayNameFlipped", m_inputStoreArrayNameFlipped,
           "Name of the input StoreArray for flipped tracks");
}

void FlippedRecoTracksMergerModule::initialize()
{
  m_inputRecoTracks.isRequired(m_inputStoreArrayName);
  m_inputRecoTracksFlipped.isRequired(m_inputStoreArrayNameFlipped);
}

void FlippedRecoTracksMergerModule::event()
{

  // loop all the recoTracks
  for (RecoTrack& recoTrack : m_inputRecoTracks) {

    // check if the recoTracks was fitted successfully
    if (not recoTrack.wasFitSuccessful()) {
      continue;
    }
    // get the related Belle2::Tracks
    Track* track = recoTrack.getRelatedFrom<Belle2::Track>();

    if (!track) continue;

    // get the cut from DB
    if (!m_flipCutsFromDB.isValid()) continue;
    double mvaFlipCut = (*m_flipCutsFromDB).getSecondCut();

    // if we should not flip the tracks: the 2nd MVA QI is nan (aka didn't pass the 1st MVA filter) or smaller than the cut
    if (isnan(recoTrack.get2ndFlipQualityIndicator()) or (recoTrack.get2ndFlipQualityIndicator() < mvaFlipCut)) continue;
    // get the related RecoTrackflipped
    RecoTrack* RecoTrackflipped =  recoTrack.getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

    if (!RecoTrackflipped) continue;

    // get the tracksflipped
    Track* trackFlipped = RecoTrackflipped->getRelatedFrom<Belle2::Track>("Tracks_flipped");
    if (!trackFlipped) continue;
    std::vector<Track::ChargedStableTrackFitResultPair> fitResultsAfter =
      trackFlipped->getTrackFitResultsByName("TrackFitResults_flipped");
    std::vector<Track::ChargedStableTrackFitResultPair> fitResultsBefore = track->getTrackFitResults();


    // Test if at least one track fit hypothesis of old track is present in the flippled track. If that is not the case
    // the update procedure fails and will cause a FATAL downstream
    bool isUpdatable = false;
    for (const auto& oldFitResult : fitResultsBefore) {
      if (std::find_if(fitResultsAfter.begin(), fitResultsAfter.end(), [&oldFitResult](Track::ChargedStableTrackFitResultPair & a) {
      return a.first == oldFitResult.first;
    }) != fitResultsAfter.end()) {
        isUpdatable = true;
        break;
      }
    }
    if (not isUpdatable) {
      continue;
    }


    //set the c_isFlippedAndRefitted bit
    track->setFlippedAndRefitted();

    // loop over the original fitResults
    for (long unsigned int index = 0; index < fitResultsBefore.size() ; index++) {
      bool updatedFitResult = false;
      for (long unsigned int index1 = 0; index1 < fitResultsAfter.size() ; index1++) {
        if (fitResultsBefore[index].first == fitResultsAfter[index1].first) {

          auto fitResultAfter  = fitResultsAfter[index1].second;
          fitResultsBefore[index].second->updateTrackFitResult(*fitResultAfter);
          updatedFitResult = true;
        }

      }
      if (not updatedFitResult) {
        fitResultsBefore[index].second->mask();
        track->setTrackFitResultIndex(fitResultsBefore[index].first, -1);
      }
    }


    const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromLastHit();

    const TVector3& currentPosition = measuredStateOnPlane.getPos();
    const TVector3& currentMomentum = measuredStateOnPlane.getMom();
    const double& currentCharge = measuredStateOnPlane.getCharge();

    // revert the charge and momentum
    recoTrack.setChargeSeed(-currentCharge);
    recoTrack.setPositionAndMomentum(currentPosition,  -currentMomentum);

    // Reverse the SortingParameters
    auto RecoHitInfos = recoTrack.getRecoHitInformations();
    for (auto RecoHitInfo : RecoHitInfos) {
      RecoHitInfo->setSortingParameter(std::numeric_limits<unsigned int>::max() - RecoHitInfo->getSortingParameter());
    }

    // swap outgoing and ingoing arm times (computed with SVD hits)
    recoTrack.swapArmTimes();

  }
}
