/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/ChooseablePathFilter.h>
#include <tracking/vxdHoughTracking/findlets/LimitedOnHitApplier.icc.h>
#include <tracking/vxdHoughTracking/findlets/PathLengthToggledApplier.icc.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

template class
Belle2::vxdHoughTracking::PathLengthToggledApplier<VXDHoughState, LimitedOnHitApplier<VXDHoughState, ChooseableFilter<PathFilterFactory>>>;
