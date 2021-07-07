/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/ChooseablePXDRelationFilter.h>
#include <tracking/ckf/pxd/filters/relations/LayerPXDRelationFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class Belle2::LayerPXDRelationFilter<ChooseableFilter<PXDPairFilterFactory>>;
