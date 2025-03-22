/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialStraightTrackCreator.h>
#include <framework/datastore/StoreArray.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ECLCluster;

  namespace TrackingUtilities {
    class CDCTrack;
    class CDCWireHit;
  }
  namespace TrackFindingCDC {

    /**
     * Generates straight axial tracks from IP to an ECL cluster.
     *
     * NOTE this might be merged with MonopoleAxialTrackFinderLegendre after debugging
     */
    class AxialStraightTrackFinder : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCTrack>;

    public:
      /// Constructor
      AxialStraightTrackFinder();

      void initialize() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method to apply the track finding.
      void apply(const std::vector<TrackingUtilities::CDCWireHit>& wireHits, std::vector<TrackingUtilities::CDCTrack>& tracks);

    private:
      /// Accessor to the ECLClusters StoreArray
      StoreArray<ECLCluster> m_storeArrayClusters{"ECLClusters"};

      /// Findlet for straight legendre pass
      AxialStraightTrackCreator m_axialStraightTrackCreator;

      // Other findlets, especially post-processing should be here.
    };
  }
}

