/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
