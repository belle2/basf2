/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
