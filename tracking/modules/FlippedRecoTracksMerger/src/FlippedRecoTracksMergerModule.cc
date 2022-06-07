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

  for (auto& recoTrack : m_inputRecoTracks) {
    // check if the recoTracks was fitted successfully
    if (not recoTrack.wasFitSuccessful()) {
      continue;
    }
    // get the related Belle2::Tracks
    Track* b2track = recoTrack.getRelatedFrom<Belle2::Track>();

    if (b2track) {
      // printing out the fit results for testing
      auto allFitRes = b2track->getTrackFitResults();
      for (auto fitRes : allFitRes) {
        B2INFO("before flipping : phi0_variance " << fitRes.second->getCov()[5]);
      }

      // if the 2ndMVA passed the cut
      if (recoTrack.get2ndFlipQualityIndicator() > m_2nd_mva_cut) {
        B2INFO(" step 3: pass the : > m_2nd_mva_cut");

        // get the related RecoTrack_flipped
        RecoTrack* RecoTrack_flipped =  recoTrack.getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

        if (RecoTrack_flipped) {
          B2INFO(" step 4: found flipped");

          // get the Tracks_flipped
          Track* b2trackFlipped = RecoTrack_flipped->getRelatedFrom<Belle2::Track>("Tracks_flipped");
          if (b2trackFlipped) {

            // printing out the fit results for testing
            allFitRes = b2trackFlipped->getTrackFitResults();
            for (auto fitRes : allFitRes) {
              B2INFO("the flipped output : phi0_variance " << fitRes.second->getCov()[5]);
            }

            // get the StoreAccessorBase for recoTrack
            auto c_PersistentDurability = BIT(21);
            bool array = (dynamic_cast<TClonesArray*>(&recoTrack) != nullptr);
            bool isPersistent = recoTrack.TestBit(c_PersistentDurability);
            DataStore::EDurability durability = isPersistent ? (DataStore::c_Persistent) : (DataStore::c_Event);

            StoreAccessorBase* RecoTracksStoreAccessorBase = new Belle2::StoreAccessorBase(m_inputStoreArrayName, durability,
                RecoTrack::Class(), array);


            // get the StoreAccessorBase for RecoTrack_flipped
            array = (dynamic_cast<TClonesArray*>(RecoTrack_flipped) != nullptr);
            isPersistent = RecoTrack_flipped->TestBit(c_PersistentDurability);
            durability = isPersistent ? (DataStore::c_Persistent) : (DataStore::c_Event);
            StoreAccessorBase* RecoTracksFlippedStoreAccessorBase = new Belle2::StoreAccessorBase(m_inputStoreArrayNameFlipped, durability,
                RecoTrack::Class(), array);

            // replace the contents of recoTrack using RecoTrack_flipped
            //     this is the key part but not working properly at the moment...
            DataStore::Instance().replaceData(*RecoTracksFlippedStoreAccessorBase, *RecoTracksStoreAccessorBase);

            // swap the Relations to Belle2::Tracks and FitResults
            RelationArray RecoTracksFlippedRelation(DataStore::relationName("Tracks_flipped", m_inputStoreArrayNameFlipped));
            RelationArray RecoTracksRelation(DataStore::relationName("Tracks", m_inputStoreArrayName));
            DataStore::Instance().replaceData(RecoTracksFlippedRelation, RecoTracksRelation);
          }

          //check  the fit results again to see if it changed or not
          B2INFO(" step 5: done ?");
          b2track = recoTrack.getRelatedFrom<Belle2::Track>();
          if (b2track) {
            allFitRes = b2track->getTrackFitResults();
            for (auto fitRes : allFitRes) {
              B2INFO("after flipping : phi0_variance " << fitRes.second->getCov()[5]);
            }
          }
        }
      }
    }
  }
}


