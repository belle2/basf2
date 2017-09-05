/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/findlets/base/TrackFitterAndDeleter.h>
#include <tracking/ckf/utilities/ResultAlgorithms.h>

#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

namespace Belle2 {
  /**
   * Findlet for loading the seeds and the hits from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   */
  template <class ASeedObject, class AHitObject>
  class CKFDataLoader : public TrackFindingCDC::Findlet<ASeedObject*, const AHitObject*> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<ASeedObject*, const AHitObject*>;

  public:
    /// Add the subfindlets
    CKFDataLoader() : Super()
    {
      this->addProcessingSignalListener(&m_hitsLoader);
      this->addProcessingSignalListener(&m_trackFitter);
    }

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_hitsLoader.exposeParameters(moduleParamList, prefix);
      m_trackFitter.exposeParameters(moduleParamList, prefix);


      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportTracks"), m_param_exportTracks,
                                    "Export the result tracks into a StoreArray.",
                                    m_param_exportTracks);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "inputRecoTrackStoreArrayName"),
                                    m_param_inputRecoTrackStoreArrayName,
                                    "StoreArray name of the input Track Store Array.",
                                    m_param_inputRecoTrackStoreArrayName);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRecoTrackStoreArrayName"),
                                    m_param_outputRecoTrackStoreArrayName,
                                    "StoreArray name of the output Track Store Array.",
                                    m_param_outputRecoTrackStoreArrayName);
    }

    /// Create the store arrays
    void initialize() override
    {
      Super::initialize();

      m_inputRecoTracks.isRequired(m_param_inputRecoTrackStoreArrayName);

      if (not m_param_exportTracks) {
        return;
      }

      m_outputRecoTracks.registerInDataStore(m_param_outputRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_outputRecoTracks);

      StoreArray<RecoTrack> relationRecoTracks(m_param_inputRecoTrackStoreArrayName);
      relationRecoTracks.registerRelationTo(m_outputRecoTracks);
    }

    /// Load in the reco tracks and the hits
    void apply(std::vector<ASeedObject*>& seeds, std::vector<const AHitObject*>& hits) override
    {
      seeds.reserve(seeds.size() + m_inputRecoTracks.getEntries());

      for (auto& item : m_inputRecoTracks) {
        seeds.push_back(&item);
      }

      m_trackFitter.apply(seeds);
      m_hitsLoader.apply(hits);
    }

    /// Store the reco tracks and the relations
    void store(std::vector<CKFResultObject<ASeedObject, AHitObject>>& results)
    {
      if (not m_param_exportTracks) {
        return;
      }

      for (const auto& result : results) {
        RecoTrack* seed = result.getSeed();
        if (not seed) {
          continue;
        }

        const auto& matchedHits = result.getHits();
        B2ASSERT("There are no hits related!", not matchedHits.empty());

        const TVector3& trackPosition = result.getPosition();
        const TVector3& trackMomentum = result.getMomentum();
        const short& trackCharge = result.getCharge();

        RecoTrack* newRecoTrack = m_outputRecoTracks.appendNew(trackPosition, trackMomentum, trackCharge);
        RecoTrackHitsAdder::addHitsToRecoTrack(matchedHits, *newRecoTrack);

        seed->addRelationTo(newRecoTrack);
      }
    }

  private:
    // Findlets
    /// Findlet for retrieving the hits
    TrackFindingCDC::StoreArrayLoader<const SpacePoint> m_hitsLoader;
    /// Findlet for fitting the tracks
    TrackFitterAndDeleter m_trackFitter;

    // Parameters
    /** Export the tracks or not */
    bool m_param_exportTracks = true;
    /** StoreArray name of the output Track Store Array */
    std::string m_param_outputRecoTrackStoreArrayName = "CKFRecoTracks";
    /** StoreArray name of the input Track Store Array */
    std::string m_param_inputRecoTrackStoreArrayName = "RecoTracks";

    // Store Arrays
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_outputRecoTracks;
    /// Input Reco Tracks Store Array
    StoreArray<RecoTrack> m_inputRecoTracks;
  };
}
