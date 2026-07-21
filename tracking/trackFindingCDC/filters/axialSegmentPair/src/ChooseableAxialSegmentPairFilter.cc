/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/ChooseableAxialSegmentPairFilter.h>

#include <tracking/trackingUtilities/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

template class TrackingUtilities::Chooseable<BaseAxialSegmentPairFilter>;
template class TrackingUtilities::ChooseableFilter<AxialSegmentPairFilterFactory>;
