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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FitlessFacetFilter::FitlessFacetFilter(bool hardCut) :
  m_param_hardCut(hardCut)
{
}

void FitlessFacetFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "hard_fitless_cut") {
    if (value == "true") {
      setHardCut(true);
    } else if (value == "false") {
      setHardCut(false);
    } else {
      Super::setParameter(key, value);
      return;
    }
    B2INFO("Filter received parameter '" << key << "' " << value);
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> FitlessFacetFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["hard_fitless_cut"] = "Switch to disallow the boarderline possible hit and "
                            "right left passage information. "
                            "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
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
