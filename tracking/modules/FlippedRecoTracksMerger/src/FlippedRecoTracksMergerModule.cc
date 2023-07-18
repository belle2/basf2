/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/FlippedRecoTracksMerger/FlippedRecoTracksMergerModule.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

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

  addParam("pxdHitsStoreArrayName", m_param_pxdHitsStoreArrayName, "StoreArray name of the input PXD hits.",
           m_param_pxdHitsStoreArrayName);
  addParam("svdHitsStoreArrayName", m_param_svdHitsStoreArrayName, "StoreArray name of the input SVD hits.",
           m_param_svdHitsStoreArrayName);
  addParam("cdcHitsStoreArrayName", m_param_cdcHitsStoreArrayName, "StoreArray name of the input CDC hits.",
           m_param_cdcHitsStoreArrayName);
  addParam("bklmHitsStoreArrayName", m_param_bklmHitsStoreArrayName, "StoreArray name of the input BKLM hits.",
           m_param_bklmHitsStoreArrayName);
  addParam("eklmHitsStoreArrayName", m_param_eklmHitsStoreArrayName, "StoreArray name of the input EKLM hits.",
           m_param_eklmHitsStoreArrayName);
}

void FlippedRecoTracksMergerModule::initialize()
{
  m_inputRecoTracks.isRequired(m_inputStoreArrayName);
  m_inputRecoTracksFlipped.isRequired(m_inputStoreArrayNameFlipped);
  m_trackFitResults.isRequired();
}

void FlippedRecoTracksMergerModule::event()
{
  // get the cut from DB
  if (!m_flipCutsFromDB.isValid()) {
    B2WARNING("DBobjects : TrackFlippingCuts not found!");
    return;
  }

  // check if the flip&refit is switched on (or off)
  if (!(*m_flipCutsFromDB).getOnOffInfo()) return;


  // loop all the recoTracks
  for (RecoTrack& recoTrack : m_inputRecoTracks) {

    // check if the recoTracks was fitted successfully
    if (not recoTrack.wasFitSuccessful()) {
      continue;
    }
    // get the related Belle2::Tracks
    Track* track = recoTrack.getRelatedFrom<Belle2::Track>();

    if (!track) continue;

    // do not change the tracks with Pt > 0.3 GeV
    auto trackFitResult = track->getTrackFitResultWithClosestMass(Const::pion);
    if (trackFitResult->getTransverseMomentum() > (*m_flipCutsFromDB).getPtCut()) continue;

    double mvaFlipCut = (*m_flipCutsFromDB).getSecondCut();

    // if we should not flip the tracks: the 2nd MVA QI is nan (aka didn't pass the 1st MVA filter) or smaller than the cut
    if (isnan(recoTrack.get2ndFlipQualityIndicator()) or (recoTrack.get2ndFlipQualityIndicator() < mvaFlipCut)) continue;
    // get the related flippedRecoTrack
    RecoTrack* flippedRecoTrack =  recoTrack.getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

    if (!flippedRecoTrack) continue;

    // get the tracksflipped
    Track* trackFlipped = flippedRecoTrack->getRelatedFrom<Belle2::Track>("Tracks_flipped");
    if (!trackFlipped) continue;
    std::vector<Track::ChargedStableTrackFitResultPair> fitResultsAfter =
      trackFlipped->getTrackFitResultsByName("TrackFitResults_flipped");
    std::vector<Track::ChargedStableTrackFitResultPair> fitResultsBefore = track->getTrackFitResults();

    //set the c_isFlippedAndRefitted bit
    track->setFlippedAndRefitted();

    // invalidate all TrackFitResults of old Track that dont exist in new Track
    for (auto fitResult : fitResultsBefore) {
      auto iterFitResult = std::find_if(fitResultsAfter.begin(),
      fitResultsAfter.end(), [&fitResult](const Track::ChargedStableTrackFitResultPair & a) { return a.first == fitResult.first;});
      // did not find this hypothesis in the new FitResults, so invalidate it
      if (iterFitResult == fitResultsAfter.end()) {
        track->setTrackFitResultIndex(fitResult.first, -1);
        fitResult.second->mask();
      }
    }


    // loop over new TrackFitResults and update or add new FitResults to the old Track
    for (auto fitResult : fitResultsAfter) {

      auto oldFitResultPairIter = std::find_if(fitResultsBefore.begin(),
      fitResultsBefore.end(), [&fitResult](const Track::ChargedStableTrackFitResultPair & a) { return a.first == fitResult.first;});

      // if old result exists update it, if not exists add a new TrackFitResult
      if (oldFitResultPairIter != fitResultsBefore.end()) {
        oldFitResultPairIter->second->updateTrackFitResult(*fitResult.second);
      } else {
        TrackFitResult* newFitResult = m_trackFitResults.appendNew();
        newFitResult->updateTrackFitResult(*fitResult.second);
        const int newTrackFitResultArrayIndex = newFitResult->getArrayIndex();
        track->setTrackFitResultIndex(fitResult.first, newTrackFitResultArrayIndex);
      }
    }

    recoTrack.flipTrackDirectionAndCharge();

    // swapping the flipped genfit::Track into the main RecoTrack, to store the new fitted values (useful for V0Finder)
    genfit::Track flipTrack = RecoTrackGenfitAccess::getGenfitTrack(*flippedRecoTrack);
    RecoTrackGenfitAccess::swapGenfitTrack(recoTrack, &flipTrack);
  }
}
