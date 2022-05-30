/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/flipQualityModule/FlipQualityModule.h>
#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>
#include <memory>

namespace Belle2 {

  REG_MODULE(FlipQuality)

  FlipQualityModule::FlipQualityModule() : Module()
  {
    setDescription("Prototype of a module which uses the MVA package");

    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("identifier", m_identifier, "The database identifier or filename which is used to load the weights during the training.",
             m_identifier);

    addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the recoTrack StoreArray.", m_recoTracksStoreArrayName);

    addParam("indexOfFlippingMVA", m_flipmva_index, "Index of flipping MVA. (1 or 2).", m_flipmva_index);
  }

  void FlipQualityModule::initialize()
  {
    // If the identifier does not end on .root or .xml, we are dealing with a database identifier
    // so we need to create a DBObjPtr, which will fetch the weightfile from the database
    B2INFO("init he FlipQualityModule");
    m_recoTracks.isRequired(m_recoTracksStoreArrayName);

    if (m_flipmva_index == 1) {
      m_recoTrackExtractor = std::make_unique<FlipRecoTrackExtractor>(m_variableSet);
    } else if (m_flipmva_index == 2) {
      m_recoTrackExtractor_2nd = std::make_unique<FlipRecoTrackExtractor_2ndmva>(m_variableSet);
    } else {
      B2INFO("no input value extractor");
    }
    // The supported methods have to be initialized once (calling it more than once is save)
    m_mvaExpert = std::make_unique<MVAExpert>(m_identifier, m_variableSet);
    m_mvaExpert->initialize();
  }

  void FlipQualityModule::beginRun()
  {

    m_mvaExpert->beginRun();

  }

  void FlipQualityModule::event()
  {
    // Just to be safe we check again if the MVA::Expert is loaded
    // It can happen that for example the database doesn't find the payload
    // and the expert ends up uninitialized.
    if (not m_mvaExpert) {
      B2ERROR("MVA Expert is not loaded! I will return 0");
      return;
    }

    // fill the dataset
    // The order must be the same as the order of the variables in general_options.m_variables
    for (RecoTrack& recoTrack : m_recoTracks) {
      // for the 1st MVA
      // we call the corresponding class and set the Qi using setFlipQualityIndicator()
      if (m_flipmva_index == 1) {
        m_recoTrackExtractor->extractVariables(recoTrack);
        float probability = m_mvaExpert->predict();
        recoTrack.setFlipQualityIndicator(probability);

      } else if (m_flipmva_index == 2) {
        // for the 2nd MVA
        // first check if the flipped track was created or not.
        // then call the corresponding class and set the 2nd Qi using set2ndFlipQualityIndicator()
        RecoTrack* RecoTrack_flipped = recoTrack.getRelatedFrom<RecoTrack>("RecoTracks_flipped");
        if (RecoTrack_flipped) {
          m_recoTrackExtractor_2nd->extractVariables(recoTrack);
          float probability = m_mvaExpert->predict();
          recoTrack.set2ndFlipQualityIndicator(probability);
        } else {
          recoTrack.set2ndFlipQualityIndicator(-999);
        }
      } else {
        B2INFO("Nothing will be saved into the flipping qi");

      }

    }

  }

} // Belle2 namespace

