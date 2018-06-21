/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialStraightTrackCreator.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ECLCluster;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;

    /**
     * Generates straight axial tracks from IP to an ECL cluster.
     *
     * NOTE this might be merged with MonopoleAxialTrackFinderLegendre after debugging
     */
    class AxialStraightTrackFinder : public Findlet<const CDCWireHit, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCTrack>;

    public:
      /// Constructor
      AxialStraightTrackFinder();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method to apply the track finding.
      void apply(const std::vector<CDCWireHit>& wireHits, std::vector<CDCTrack>& tracks);

    private: // findlets

      // Findlet for straight legendre pass
      AxialStraightTrackCreator m_axialStraightTrackCreator;

      // Other findlets, especially post-processing should be here.
    };
  }
}

