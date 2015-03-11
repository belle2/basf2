/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/FitlessFacetFilter.h"

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FitlessFacetFilter::FitlessFacetFilter(bool hard) :
  m_hard(hard)
{
}



CellState FitlessFacetFilter::isGoodFacet(const CDCRecoFacet& facet)
{
  CDCRecoFacet::Shape shape = facet.getShape();

  const RightLeftInfo& startRLInfo = facet.getStartRLInfo();
  const RightLeftInfo& middleRLInfo = facet.getMiddleRLInfo();
  const RightLeftInfo& endRLInfo = facet.getEndRLInfo();

  bool selectFitless = true;

  if (shape == facet.ORTHO_CW or shape == facet.ORTHO_CCW) {
    selectFitless = middleRLInfo * shape > 0 and (middleRLInfo != startRLInfo or
                                                  middleRLInfo != endRLInfo);

  } else if (shape ==  facet.META_CW or shape ==  facet.META_CCW) {

    selectFitless = (middleRLInfo * shape > 0 and
                     (middleRLInfo != startRLInfo or middleRLInfo != endRLInfo));

    if (not m_hard) {
      selectFitless |= (middleRLInfo == startRLInfo and middleRLInfo == endRLInfo);
    }

  } else if (shape ==  facet.PARA) {
    selectFitless = (startRLInfo == middleRLInfo) or (middleRLInfo == endRLInfo);

  } else {
    selectFitless = false;

  }

  return selectFitless ? 3 : NOT_A_CELL;
}
