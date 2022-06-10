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

  // record the index of FitResults in store Array
  unsigned int lastFitResultIndexInArray = -1;
  unsigned int fitResultsIndexFlipped = -1;

  // get the last index of the fitresults
  for (int ireco = 0; ireco <  m_inputRecoTracks.getEntries(); ireco++) {
    RecoTrack* recoTrack = m_inputRecoTracks[ireco];
    // check if the recoTracks was fitted successfully
    if (not recoTrack->wasFitSuccessful()) {
      continue;
    }

    Track* b2track = recoTrack->getRelatedFrom<Belle2::Track>();
    if (b2track) {
      lastFitResultIndexInArray += b2track->getNumberOfFittedHypotheses();
    }
  }

  // the flipped fitResults' index in the storeArray
  fitResultsIndexFlipped = lastFitResultIndexInArray;

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

      // if the 2ndMVA is not -999 (aka passed the 1st MVA)
      if (recoTrack->get2ndFlipQualityIndicator() != -999) {

        // get the related RecoTrack_flipped
        RecoTrack* RecoTrack_flipped =  recoTrack->getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");
        // get the Tracks_flipped
        Track* b2trackFlipped = RecoTrack_flipped->getRelatedFrom<Belle2::Track>("Tracks_flipped");
        // get the flipped fitResults
        std::vector<Track::ChargedStableTrackFitResultPair> fitResultsAfter = b2trackFlipped->getTrackFitResults();

        // if the 2MVA existed and passed the selection
        if (recoTrack->get2ndFlipQualityIndicator() > m_2nd_mva_cut) {
          // assign the Index of fitResult in storeArray to Flipped fitResults Index
          // if the original Track has more fitResults, the rest of them will not be changed --> to be updated
          for (long unsigned int index = 0; index < fitResultsAfter.size(); index++) {
            auto fitResultAfter  = fitResultsAfter[index];
            fitResultsIndexFlipped ++;
            b2track -> setTrackFitResultIndex(fitResultAfter.first, fitResultsIndexFlipped);
            //B2INFO("the flipped output : phi0_variance " << fitResultAfter.second->getCov()[5]);
          }

          // update the contents in RecoTracks
          const auto& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromLastHit();
          TVector3 currentPosition = measuredStateOnPlane.getPos();
          TVector3 currentMomentum = measuredStateOnPlane.getMom();
          double currentCharge = measuredStateOnPlane.getCharge();

          // revert the charge and momentum
          recoTrack->setChargeSeedOnly(-currentCharge);
          recoTrack->setPositionAndMomentumOnly(currentPosition,  -currentMomentum);

          // Can we only reverse the SortingParameters?
          auto RecoHitInfos = recoTrack->getRecoHitInformations();
          unsigned int temp = 0xffffffff;
          for (auto RecoHitInfo : RecoHitInfos) {
            RecoHitInfo->setSortingParameter(temp - RecoHitInfo->getSortingParameter());
          }

        } else {
          // in case the track was flipped and had fitResults saved
          fitResultsIndexFlipped += fitResultsAfter.size() ;

        }

      }
    }
  }
}
