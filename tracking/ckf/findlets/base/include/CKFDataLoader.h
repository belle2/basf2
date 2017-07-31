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
      this->addProcessingSignalListener(&m_tracksLoader);
      this->addProcessingSignalListener(&m_hitsLoader);
      this->addProcessingSignalListener(&m_trackFitter);
    }

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_tracksLoader.exposeParameters(moduleParamList, prefix);
      m_hitsLoader.exposeParameters(moduleParamList, prefix);
      m_trackFitter.exposeParameters(moduleParamList, prefix);
    }

    /// Do the track/hit finding/merging.
    void apply(std::vector<ASeedObject*>& seeds, std::vector<const AHitObject*>& hits) override
    {
      m_tracksLoader.apply(seeds);
      m_trackFitter.apply(seeds);

      m_hitsLoader.apply(hits);
    }

  private:
    // Findlets
    /// Findlet for retrieving the tracks
    TrackFindingCDC::StoreArrayLoader<RecoTrack> m_tracksLoader;
    /// Findlet for retrieving the hits
    TrackFindingCDC::StoreArrayLoader<const SpacePoint> m_hitsLoader;
    /// Findlet for fitting the tracks
    TrackFitterAndDeleter m_trackFitter;
  };
}
