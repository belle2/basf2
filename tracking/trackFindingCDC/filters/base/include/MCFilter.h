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
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// MC Filter Type using a VarSet and the truth variable in it.
    template<class ATruthVarSet>
    using MCFilter = TruthVarFilter<ATruthVarSet>;
  }
}
