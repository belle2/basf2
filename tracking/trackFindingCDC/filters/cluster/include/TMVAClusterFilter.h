/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "BaseClusterFilter.h"

#include <tracking/trackFindingCDC/tmva/Expert.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHitCluster;
    class Expert;
    class CDCWireTopology;

    class TMVAClusterFilter: public BaseClusterFilter {

      struct TMVAVariables {
        float superlayerID;
        float size;
        float totalNNeighbors;
        float meanNNeighbors;

        float totalDriftLength;
        float meanDriftLength;
        float varianceDriftLength;

        float totalInnerDistance;
        float meanInnerDistance;
        float isStereo;
        float distanceToSuperlayerCenter;
      };

    public:
      /// Constructor of the filter.
      TMVAClusterFilter();

      /// Initialize the filter before event processing
      virtual void initialize() override;

      /// Terminate the filter after event processing
      virtual void terminate() override;

      /// Set a map of parameter key and string values. Meaning depends on the specific implementation.
      virtual void setParameters(const std::map<std::string, std::string>& parameterMap) override;

    public:
      virtual CellWeight isGoodCluster(const CDCWireHitCluster& cluster) override final;

    private:
      /// Before using the TMVA we should set the variables calculated from the cluster
      void setVariables(const CDCWireHitCluster& cluster,
                        struct TMVAVariables& tmvaVariables);

      // Prepare the super layer center array with information coming from the CDCWireTopology object.
      void prepareSuperLayerCenterArray(const TrackFindingCDC::CDCWireTopology& wireTopology);

      /// The cut on the TMVA output.
      double m_param_cut;

      /// TMVA Expert to decide if a cluster is background or not.
      Expert m_expert;

      /// TMVA Variables on which the expert may base its decision.
      TMVAVariables m_tmvaVariables;

      /// The cylinder radius of the super layer centers
      std::vector<double> m_superLayerCenters;
    };

  }

}
