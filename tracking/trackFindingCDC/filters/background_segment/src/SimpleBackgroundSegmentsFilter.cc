/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/background_segment/SimpleBackgroundSegmentsFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CellWeight SimpleBackgroundSegmentsFilter::operator()(const CDCRecoSegment2D& segment)
{
  Super::operator()(segment);

  // TODO

  // Means: yes
  return 1.0;
}
