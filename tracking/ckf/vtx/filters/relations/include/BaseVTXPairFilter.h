/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>
#include <tracking/ckf/vtx/entities/CKFToVTXState.h>

namespace Belle2 {
  /// Base filter for CKF VTX states
  using BaseVTXPairFilter = TrackFindingCDC::Filter<std::pair<const CKFToVTXState*, const CKFToVTXState*>>;
}
