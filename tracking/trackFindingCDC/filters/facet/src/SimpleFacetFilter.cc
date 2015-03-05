/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SimpleFacetFilter.h"

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;





SimpleFacetFilter::SimpleFacetFilter():
  m_allowedDeviationCos(cos(PI / 180.0 * 9))
{
}





SimpleFacetFilter::SimpleFacetFilter(FloatType allowedDeviationCos):
  m_allowedDeviationCos(allowedDeviationCos)
{
}





CellState SimpleFacetFilter::isGoodFacet(const CDCRecoFacet& facet)
{

  facet.adjustLines();

  const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEnd    = facet.getStartToEndLine();
  const ParameterLine2D& middleToEnd   = facet.getMiddleToEndLine();

  const FloatType startCos  = startToMiddle.tangential().cosWith(startToEnd.tangential());
  // const FloatType middleCos = startToMiddle.tangential().cosWith(middleToEnd.tangential());
  const FloatType endCos = startToEnd.tangential().cosWith(middleToEnd.tangential());

  /* cut on the angle of */
  if (startCos > m_allowedDeviationCos and endCos > m_allowedDeviationCos) {

    //Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3;

  } else {

    //B2DEBUG(200,"Rejected facet because flight directions do not match");
    return NOT_A_CELL;

  }

}
