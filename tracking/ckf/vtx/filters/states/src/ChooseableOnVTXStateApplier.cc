/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/states/ChooseableOnVTXStateApplier.h>
#include <tracking/ckf/general/findlets/LimitedOnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LayerToggledApplier.icc.h>
#include <tracking/ckf/vtx/entities/CKFToVTXState.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class
Belle2::LayerToggledApplier<CKFToVTXState, LimitedOnStateApplier<CKFToVTXState, ChooseableFilter<VTXStateFilterFactory>>>;
