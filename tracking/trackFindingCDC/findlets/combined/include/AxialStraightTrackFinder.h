/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialStraightTrackCreator.h>
#include <framework/datastore/StoreArray.h>

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

      void initialize() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method to apply the track finding.
      void apply(const std::vector<CDCWireHit>& wireHits, std::vector<CDCTrack>& tracks);

    private:
      /// Accessor to the ECLClusters StoreArray
      StoreArray<ECLCluster> m_storeArrayClusters{"ECLClusters"};

      /// Findlet for straight legendre pass
      AxialStraightTrackCreator m_axialStraightTrackCreator;

      // Other findlets, especially post-processing should be here.
    };
  }
}

