/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/topology/EWirePosition.h>
#include <cdc/geometry/CDCGeometryPar.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Translate the wire position enum to the cdc version
    CDC::CDCGeometryPar::EWirePosition toCDC(EWirePosition wirePosition);

    /// Translate the cdc version to the wire position enum
    EWirePosition fromCDC(CDC::CDCGeometryPar::EWirePosition wirePosSet);
  }
}
