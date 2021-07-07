/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

namespace Belle2 {
  class ECLCluster;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

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

      /// Execute one pass over given clusters and wirehits and create tracks
      void apply(const std::vector<const ECLCluster*>& eclClusters,
                 const std::vector<const CDCWireHit*>& axialWireHits,
                 std::vector<CDCTrack>& tracks) final;

    private:
      /// Search for hits compatible with given trajectory
      std::vector<const CDCWireHit*> search(const std::vector<const CDCWireHit*>& axialWireHits,
                                            const CDCTrajectory2D& guidingTrajectory2D);

    private:
      /// ECL cluster energy threshold
      float m_param_minEnergy = 0.050; //GeV

      /// Track number of hits threshold
      unsigned int m_param_minNHits = 5;

      /// Maximum distance from hits to the track
      float m_param_maxDistance = 0.5; //cm, NOTE for conventional tracking it is 0.2
    };
  }
}
