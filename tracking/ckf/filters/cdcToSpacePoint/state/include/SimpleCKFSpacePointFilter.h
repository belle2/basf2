/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateFilter.h>

namespace Belle2 {
  class ModuleParamList;

  /// A very simple filter for all space points. Should be overloaded for the different cases (SVD, PXD).
  class SimpleCKFSpacePointFilter : public BaseCKFCDCToSpacePointStateFilter {
  public:
    /// Expose the parameters to the module
    void exposeParameters(ModuleParamList* moduleParamList,
                          const std::string& prefix) final;

    /// Set the cached B field
    void beginRun() final;

  protected:
    /// Parameter: make hit jumps possible (missing hits on a layer)
    int m_param_hitJumpingUpTo = 1;

    /// Cache for the B field at the IP
    double m_cachedBField;

    /// Helper function to check for the number of holes and overlap hits
    bool checkOverlapAndHoles(const BaseCKFCDCToSpacePointStateFilter::Object& currentState);
  };
}
