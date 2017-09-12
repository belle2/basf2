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
