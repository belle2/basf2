/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/results/ChooseablePXDResultFilter.h>
#include <tracking/trackingUtilities/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;

template class TrackingUtilities::Chooseable<BasePXDResultFilter>;
template class TrackingUtilities::ChooseableFilter<PXDResultFilterFactory>;