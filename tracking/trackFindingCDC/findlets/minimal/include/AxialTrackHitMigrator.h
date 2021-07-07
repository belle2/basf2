/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCTrack;

    /// Exchanges hits between axial tracks based on their distance to the respective trajectory.
    class AxialTrackHitMigrator : public Findlet<const CDCWireHit* const, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit* const, CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Do the hit migration
      void apply(const std::vector<const CDCWireHit*>& axialWireHits,
                 std::vector<CDCTrack>& axialTracks) final;

    private:
      /// Parameter : Distance for a hit to be removed
      double m_param_dropDistance = 0.20;

      /// Parameter : Distance for a hit to be added
      double m_param_addDistance = 0.25;
    };
  }
}
