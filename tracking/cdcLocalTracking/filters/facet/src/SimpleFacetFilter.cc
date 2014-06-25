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

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

SimpleFacetFilter::SimpleFacetFilter(): m_allowedDeviationCos(cos(PI / 180.0 * 9)) {;}
SimpleFacetFilter::SimpleFacetFilter(FloatType allowedDeviationCos): m_allowedDeviationCos(allowedDeviationCos) {;}

SimpleFacetFilter::~SimpleFacetFilter() {;}



CellState SimpleFacetFilter::isGoodFacet(const CDCRecoFacet& facet) const
{

  facet.adjustLines();

  const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEnd    = facet.getStartToEndLine();
  const ParameterLine2D& middleToEnd   = facet.getMiddleToEndLine();

  /* cut on the angle of */
  if (startToMiddle.tangential().cosWith(startToEnd.tangential()) > m_allowedDeviationCos and
      startToEnd.tangential().cosWith(middleToEnd.tangential()) > m_allowedDeviationCos and
      startToMiddle.tangential().cosWith(middleToEnd.tangential()) > m_allowedDeviationCos) {

    //Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3;

  } else {

    //B2DEBUG(200,"Rejected facet because flight directions do not match");
    return NOT_A_CELL;

  }

}



void SimpleFacetFilter::clear() const {;}


void SimpleFacetFilter::initialize()
{
}


void SimpleFacetFilter::terminate()
{
}



