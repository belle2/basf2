/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/facet/SimpleFacetFilter.h>

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;





SimpleFacetFilter::SimpleFacetFilter():
  m_fitlessFacetFilter(true),
  m_param_deviationCosCut(cos(PI / 180.0 * 9))
{
}





SimpleFacetFilter::SimpleFacetFilter(FloatType deviationCosCut):
  m_fitlessFacetFilter(true),
  m_param_deviationCosCut(deviationCosCut)
{
}

void SimpleFacetFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "deviation_cos_cut") {
    m_param_deviationCosCut = stod(value);
    B2INFO("Filter received parameter '" << key << "' " << value);
  } else {
    m_fitlessFacetFilter.setParameter(key, value);
  }
}

std::map<std::string, std::string> SimpleFacetFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = m_fitlessFacetFilter.getParameterDescription();
  des["deviation_cos_cut"] = "Acceptable deviation cosine in the angle of adjacent tangents to the "
                             "drift circles.";
  return des;
}

CellWeight SimpleFacetFilter::operator()(const CDCFacet& facet)
{
  CellWeight fitlessWeight = m_fitlessFacetFilter(facet);
  if (isNotACell(fitlessWeight)) return NOT_A_CELL;

  facet.adjustLines();

  const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEnd    = facet.getStartToEndLine();
  const ParameterLine2D& middleToEnd   = facet.getMiddleToEndLine();

  const FloatType startCos  = startToMiddle.tangential().cosWith(startToEnd.tangential());
  // const FloatType middleCos = startToMiddle.tangential().cosWith(middleToEnd.tangential());
  const FloatType endCos = startToEnd.tangential().cosWith(middleToEnd.tangential());

  /* cut on the angle of */
  if (startCos > m_param_deviationCosCut and endCos > m_param_deviationCosCut) {

    //Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3;

  } else {

    //B2DEBUG(200,"Rejected facet because flight directions do not match");
    return NOT_A_CELL;

  }

}
