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

// This header file is deprecated
// Instead use one of the following headers depending on the *minimal* needs of your use.
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Convience template to create a filter returning on variable from a set of variables.
    template <class ATruthVarSet>
    using NamedChoosableVarSetFilter = ChoosableFromVarSetFilter<ATruthVarSet>;
  }
}
