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

#include <tracking/trackFindingCDC/varsets/CDCWireHitClusterVarSet.h>
#include <tracking/trackFindingCDC/tmva/Expert.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class Expert;

    /// Background cluster detection based on TMVA.
    class TMVAClusterFilter: public BaseClusterFilter {

    public:
      /// Constructor of the filter.
      TMVAClusterFilter();

      /// Initialize the filter before event processing.
      virtual void initialize() override;

      /// Terminate the filter after event processing.
      virtual void terminate() override;

      /// Set a map of parameter key and string values. Meaning depends on the specific implementation.
      virtual void setParameters(const std::map<std::string, std::string>& parameterMap) override;

    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual CellWeight isGoodCluster(const CDCWireHitCluster& cluster) override final;

    private:
      /// The cut on the TMVA output.
      double m_param_cut;

      /// TMVA Expert to decide if a cluster is background or not.
      Expert m_expert;

      /// VarSet to generate the variables from the cluster.
      CDCWireHitClusterVarSet m_varset;
    };
  }
}
