/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/TMVAClusterFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TMVAClusterFilter::TMVAClusterFilter()
  : Super("BackgroundHitFinder", 0.2)
{
}
