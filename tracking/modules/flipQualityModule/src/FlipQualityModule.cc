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
  /// Register the Module
  REG_MODULE(FlipQuality);
  ///  Implementation
  FlipQualityModule::FlipQualityModule() : Module()
  {
    setDescription("This Module is meant to apply the (index 1 or 2) mva based filter to determine if a recoTracks should be flipped");

    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("identifier", m_identifier, "The database identifier or filename which is used to load the weights during the training.",
             m_identifier);

    addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the recoTrack StoreArray.", m_recoTracksStoreArrayName);

    addParam("indexOfFlippingMVA", m_flipMVAIndex, "Index of flipping MVA. (1 or 2).", m_flipMVAIndex);
  }

  void FlipQualityModule::initialize()
  {
    // If the identifier does not end on .root or .xml, we are dealing with a database identifier
    // so we need to create a DBObjPtr, which will fetch the weightfile from the database
    m_recoTracks.isRequired(m_recoTracksStoreArrayName);

    if (m_flipMVAIndex == 1) {
      m_recoTrackExtractor = std::make_unique<FlipRecoTrackExtractor>(m_variableSet);
    } else if (m_flipMVAIndex == 2) {
      m_recoTrackExtractor2nd = std::make_unique<FlipRecoTrackExtractor2nd>(m_variableSet);
    } else {
      B2FATAL("no input value extractor!");
    }
    // The supported methods have to be initialized once (calling it more than once is save)
    m_mvaExpert = std::make_unique<MVAExpert>(m_identifier, m_variableSet);
    m_mvaExpert->initialize();
  }

  void FlipQualityModule::beginRun()
  {

    // Just to be safe we check if the MVA::Expert is loaded
    if (not m_mvaExpert) {
      B2FATAL("MVA Expert is not loaded!");
    }
    m_mvaExpert->beginRun();

  }

  void FlipQualityModule::event()
  {

    // fill the dataset
    // The order must be the same as the order of the variables in general_options.m_variables
    for (RecoTrack& recoTrack : m_recoTracks) {
      // for the 1st MVA
      // we call the corresponding class and set the Qi using setFlipQualityIndicator()
      if (m_flipMVAIndex == 1) {
        m_recoTrackExtractor->extractVariables(recoTrack);
        float probability = m_mvaExpert->predict();
        recoTrack.setFlipQualityIndicator(probability);

      } else if (m_flipMVAIndex == 2) {
        // for the 2nd MVA
        // first check if the flipped track was created or not.
        // then call the corresponding class and set the 2nd Qi using set2ndFlipQualityIndicator()
        RecoTrack* RecoTrackflipped = recoTrack.getRelatedFrom<RecoTrack>("RecoTracks_flipped");
        if (RecoTrackflipped) {
          m_recoTrackExtractor2nd->extractVariables(recoTrack);
          float probability = m_mvaExpert->predict();
          recoTrack.set2ndFlipQualityIndicator(probability);
        }
      } else {
        B2DEBUG(20, "Nothing will be saved into the flipping qi");

      }

    }

  }

} // Belle2 namespace

