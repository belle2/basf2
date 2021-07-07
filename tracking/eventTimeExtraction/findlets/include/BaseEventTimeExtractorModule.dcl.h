/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/eventTimeExtraction/findlets/TrackSelector.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/findlets/wrapper/FindletStoreArrayInput.h>

namespace Belle2 {
  class RecoTrack;

  /// Base class for most of the time extraction modules doing a track selection beforehand.
  template<class AFindlet>
  class BaseEventTimeExtractorModuleFindlet final : public TrackFindingCDC::Findlet<RecoTrack*> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Add the subfindlet as listener
    BaseEventTimeExtractorModuleFindlet();

    /// Expose our parameters to the super module
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Apply the findlets
    void apply(std::vector<RecoTrack*>& recoTracks) override final;

  private:
    /// Select the tracks before applying the extraction
    TrackSelector m_trackSelector;
    /// The findlet for the extraction
    AFindlet m_findlet;
  };

  /// Alias for the event time extraction module
  template <class AFindlet>
  using EventTimeExtractorModule = TrackFindingCDC::FindletModule <
                                   TrackFindingCDC::FindletStoreArrayInput<BaseEventTimeExtractorModuleFindlet<AFindlet> >>;
}
