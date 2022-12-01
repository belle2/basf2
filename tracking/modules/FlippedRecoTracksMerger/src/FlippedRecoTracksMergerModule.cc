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

    recoTrack.flipTrackDirectionAndCharge();
    // Initialise the TrackFitter to refit the recoTrack and provide a valid genfit state
    TrackFitter fitter(m_param_pxdHitsStoreArrayName, m_param_svdHitsStoreArrayName, m_param_cdcHitsStoreArrayName,
                       m_param_bklmHitsStoreArrayName, m_param_eklmHitsStoreArrayName);
    // PDG code 211 for pion is enough to get a valid track fit with a valid genfit::MeasuredStateOnPlane
    Const::ChargedStable particleUsedForFitting(211);
    fitter.fit(recoTrack, particleUsedForFitting);
  }
}
