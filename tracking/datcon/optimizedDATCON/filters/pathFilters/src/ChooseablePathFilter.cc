/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ChooseablePathFilter.h>
#include <tracking/datcon/optimizedDATCON/findlets/LimitedOnHitApplier.icc.h>
#include <tracking/datcon/optimizedDATCON/findlets/PathLengthToggledApplier.icc.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class
Belle2::PathLengthToggledApplier<HitData, LimitedOnHitApplier<HitData, ChooseableFilter<PathFilterFactory>>>;
