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

namespace Belle2 {
  class ECLCluster;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    class Vector3D;

    /**
     * A findlet that searches for straight tracks in CDC coming form IP to an ECL cluster.
     */
    class AxialStraightTrackCreator : public Findlet<const ECLCluster* const, const CDCWireHit* const, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const ECLCluster* const, const CDCWireHit* const, CDCTrack>;

    public:
      /// Constructor
      AxialStraightTrackCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialisation before the event processing starts
      void initialize() final;

      /// Execute one pass over a quad tree
      void apply(const std::vector<const ECLCluster*>& eclClusters,
                 const std::vector<const CDCWireHit*>& axialWireHits,
                 std::vector<CDCTrack>& tracks) final;

    private:
      // Search for hits in given direction
      void search(const std::vector<const CDCWireHit*>& axialWireHits, const Vector3D& endPosition);

    private:
      // ECL cluster energy threshold
      float m_param_minEnergy = 0.0;

      // Track number of hits threshold
      float m_param_minNHits = 0.0;
    };
  }
}
