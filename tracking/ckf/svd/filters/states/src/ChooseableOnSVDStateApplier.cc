/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/ChooseableOnSVDStateApplier.h>
#include <tracking/ckf/general/findlets/LimitedOnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LayerToggledApplier.icc.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <tracking/trackingUtilities/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackingUtilities;

template class
Belle2::LayerToggledApplier<CKFToSVDState, LimitedOnStateApplier<CKFToSVDState, ChooseableFilter<SVDStateFilterFactory>>>;