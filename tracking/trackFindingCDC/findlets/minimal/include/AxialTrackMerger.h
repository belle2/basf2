/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Oliver Frost                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

    /// Findlet implementing the merging of axial tracks found in the legendre tree search
    class AxialTrackMerger : public Findlet<CDCTrack&, const CDCWireHit* const> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&, const CDCWireHit* const>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Merge tracks together. Allows for axial hits to be added as it may see fit.
      void apply(std::vector<CDCTrack>& axialTracks, const std::vector<const CDCWireHit*>& axialWireHits) final;

    private:
      /// Parameter : Minimal fit probability of the common fit of two tracks to be eligible for merging
      double m_param_minFitProb = 0.85;
    };
  }
}
