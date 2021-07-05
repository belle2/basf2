/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/ChooseableWireHitFilter.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::Chooseable<BaseWireHitFilter>;
template class TrackFindingCDC::ChooseableFilter<WireHitFilterFactory>;
