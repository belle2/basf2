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
  fitResultsIndexFlipped = lastFitResultIndexInArray + 1;

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
      // printing out the fit results for testing
      //fitResultsIndex += b2track->getNumberOfFittedHypotheses();
      //auto allFitRes = b2track->getTrackFitResults();
      //for (auto fitRes : allFitRes) {
      //  B2INFO("before flipping : phi0_variance " << fitRes.second->getCov()[5]);
      //}

      // if the 2ndMVA passed the cut
      if (recoTrack->get2ndFlipQualityIndicator() > m_2nd_mva_cut) {
        //B2INFO(" step 3: pass the : > m_2nd_mva_cut");

        // get the related RecoTrack_flipped
        RecoTrack* RecoTrack_flipped =  recoTrack->getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

        if (RecoTrack_flipped) {

          // get the Tracks_flipped
          Track* b2trackFlipped = RecoTrack_flipped->getRelatedFrom<Belle2::Track>("Tracks_flipped");

          if (b2trackFlipped) {

            // printing out the fit results for testing
            //allFitRes = b2trackFlipped->getTrackFitResults();
            //for (auto fitRes : allFitRes) {
            //  B2INFO("the flipped output : phi0_variance " << fitRes.second->getCov()[5]);
            //}

            //TVector3 currentPosition , currentMomentum;
            //double currentCharge = 0;

            //const auto& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromLastHit();
            //currentPosition = measuredStateOnPlane.getPos();
            //currentMomentum = measuredStateOnPlane.getMom();
            //currentCharge = measuredStateOnPlane.getCharge();

            //// revert the charge and momentum
            //recoTrack->setChargeSeedOnly(-currentCharge);
            //recoTrack->setPositionAndMomentumOnly(currentPosition,  -currentMomentum);

            //// Can we only reverse the SortingParameters?
            //auto RecoHitInfos = recoTrack->getRecoHitInformations();
            //unsigned int temp = 0xffffffff;
            //for (auto RecoHitInfo : RecoHitInfos) {
            //  RecoHitInfo->setSortingParameter(temp - RecoHitInfo->getSortingParameter());
            //}

            //std::vector<Track::ChargedStableTrackFitResultPair> fitResultsBefore = b2track->getTrackFitResults();
            std::vector<Track::ChargedStableTrackFitResultPair> fitResultsAfter = b2trackFlipped->getTrackFitResults();

            //b2track->getTrackFitResults().clear();
            //Problem to be fixed:
            //    if the original Track has 3 fitResults but the flipped one has 2.
            //    the last fitResults will not be changed with the following code
            for (long unsigned int index = 0; index <  fitResultsAfter.size(); index++) {
              auto fitResultAfter  = fitResultsAfter[index];
              b2track -> setTrackFitResultIndex(fitResultAfter.first, fitResultsIndexFlipped + index);
            }
          }

          //check  the fit results again to see if it changed or not
          //B2INFO(" step 5: done ?");
          //b2track = recoTrack->getRelatedFrom<Belle2::Track>();
          //if (b2track) {
          //  allFitRes = b2track->getTrackFitResults();
          //  for (auto fitRes : allFitRes) {
          //    B2INFO("after flipping : phi0_variance " << fitRes.second->getCov()[5]);
          //  }
          //}
        }
      }
    }
  }
}


