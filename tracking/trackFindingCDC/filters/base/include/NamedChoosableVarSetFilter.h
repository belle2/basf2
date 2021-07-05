/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
