/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/facet/FitlessFacetFilter.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FitlessFacetFilter::FitlessFacetFilter(bool hardCut) :
  m_param_hardCut(hardCut)
{
}

void FitlessFacetFilter::exposeParameters(ModuleParamList* moduleParamList,
                                          const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "hardCut"),
                                m_param_hardCut,
                                "Switch to disallow the boarderline possible hit and "
                                "right left passage information.",
                                m_param_hardCut);
}

CellState FitlessFacetFilter::operator()(const CDCFacet& facet)
{
  using EShape = CDCFacet::EShape;
  EShape shape = facet.getShape();

  const ERightLeft startRLInfo = facet.getStartRLInfo();
  const ERightLeft middleRLInfo = facet.getMiddleRLInfo();
  const ERightLeft endRLInfo = facet.getEndRLInfo();

  bool selectFitless = true;

  if (shape == EShape::c_OrthoCW or
      shape == EShape::c_OrthoCCW) {
    selectFitless = middleRLInfo * shape > 0 and (middleRLInfo != startRLInfo or
                                                  middleRLInfo != endRLInfo);

  } else if (shape == EShape::c_MetaCW or
             shape == EShape::c_MetaCCW) {

    selectFitless = (middleRLInfo * shape > 0 and
                     (middleRLInfo != startRLInfo or middleRLInfo != endRLInfo));

    if (not m_param_hardCut) {
      selectFitless |= (middleRLInfo == startRLInfo and middleRLInfo == endRLInfo);
    }

  } else if (shape == EShape::c_Para) {
    selectFitless = (startRLInfo == middleRLInfo) or (middleRLInfo == endRLInfo);

  } else {
    selectFitless = false;

  }

  return selectFitless ? 3 : NAN;
}
