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
  setDescription("Replace the original RecoTrack with the flipped one if the 2nd Flip qi passed the selection  ");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("MVA2nd_cut", m_2nd_mva_cut, "the cut for 2nd flip mva",  m_2nd_mva_cut);
}

void FlippedRecoTracksMergerModule::initialize()
{

  //StoreArray<RecoTrack> m_inputRecoTracks(m_inputStoreArrayName);
  //m_outputRecoTracks.registerInDataStore(m_outputStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  //RecoTrack::registerRequiredRelations(m_outputRecoTracks);

  //m_outputRecoTracks.registerRelationTo(m_inputRecoTracks);

  //if (m_tracks.optionalRelationTo(m_inputRecoTracks)) {
  //  m_tracks.registerRelationTo(m_outputRecoTracks);
  //}
}

void FlippedRecoTracksMergerModule::event()
{

  Belle2::StoreArray<RecoTrack> m_inputRecoTracks(m_inputStoreArrayName);
  //B2INFO(" step 1: FlippedRecoTracksMergerModule::event()");
  for (const auto& recoTrack : m_inputRecoTracks) {
    // B2INFO(" setp 2: RecoTrack& recoTrack : m_inputRecoTracks");
    //if (m_param_onlyFittedTracks and not recoTrack.wasFitSuccessful()) {
    //  continue;
    //}
    //Track* b2track = recoTrack.getRelatedFrom<Belle2::Track>();
    //if (b2track){
    //    auto allFitRes = b2track->getTrackFitResults();
    //    for (auto fitRes: allFitRes){
    //       B2INFO("before flipping : phi0_variance "<< fitRes.second->getCov()[5]);
    //    }
    //}
    if (recoTrack.get2ndFlipQualityIndicator() > m_2nd_mva_cut) {
      B2INFO(" step 3: pass the : > m_2nd_mva_cut");
      const RecoTrack* RecoTrack_flipped =  recoTrack.getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

      if (RecoTrack_flipped) {
        B2INFO(" step 4: found flipped");
        // how to use the replaceData() here???
        //DataStore::Instance().replaceData(RecoTrack_flipped, recoTrack);
      }
    }
  }
}


