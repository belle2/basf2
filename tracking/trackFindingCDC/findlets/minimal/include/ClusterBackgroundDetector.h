/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCWireHitCluster;

    /// Marks clusters as background based on a background measure
    class ClusterBackgroundDetector : public Findlet<CDCWireHitCluster&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHitCluster&>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterBackgroundDetector();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main algorithm applying the cluster background detection
      void apply(std::vector<CDCWireHitCluster>& outputClusters) final;

    private:
      /// Chooseable cluster filter to be used to filter background
      ChooseableClusterFilter m_clusterFilter;
    };
  }
}
