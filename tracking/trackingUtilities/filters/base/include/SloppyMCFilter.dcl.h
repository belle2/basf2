/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/SloppyFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Sloppy MC Filter Type using a VarSet and the truth variable in it. It will return true for correct ones and
     * based on the scale factor also sometimes for false ones.
     */
    template<class ATruthVarSet>
    using SloppyMCFilter = Sloppy<TruthVarFilter<ATruthVarSet> >;
  }
}
