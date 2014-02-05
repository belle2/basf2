/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "CDCGenHit.h"

#include "CDCWireHit.h"
#include "CDCRecoHit2D.h"
#include "CDCRecoTangent.h"
#include "CDCRecoFacet.h"

#include "CDCRecoHit3D.h"



namespace Belle2 {
  namespace CDCLocalTracking {

    /// Wire and the dummy generic hits shall be coaligned
    bool operator<(const CDCGenHit& genHit, const CDCWire& wire) { return genHit.getWire() < wire; }

    /// Wire and the dummy generic hits shall be coaligned
    bool operator<(const CDCWire& wire, const CDCGenHit& genHit) { return wire < genHit.getWire(); }


  } //end namespace CDCLocalTracking
} //end namespace Belle2
