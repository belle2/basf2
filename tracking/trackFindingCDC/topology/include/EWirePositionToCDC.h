/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
