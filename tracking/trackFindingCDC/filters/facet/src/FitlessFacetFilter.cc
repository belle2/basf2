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
  CDCFacet::Shape shape = facet.getShape();

  const ERightLeft startRLInfo = facet.getStartRLInfo();
  const ERightLeft middleRLInfo = facet.getMiddleRLInfo();
  const ERightLeft endRLInfo = facet.getEndRLInfo();

  bool selectFitless = true;

  if (shape == facet.ORTHO_CW or shape == facet.ORTHO_CCW) {
    selectFitless = middleRLInfo * shape > 0 and (middleRLInfo != startRLInfo or
                                                  middleRLInfo != endRLInfo);

  } else if (shape ==  facet.META_CW or shape ==  facet.META_CCW) {

    selectFitless = (middleRLInfo * shape > 0 and
                     (middleRLInfo != startRLInfo or middleRLInfo != endRLInfo));

    if (not m_param_hardCut) {
      selectFitless |= (middleRLInfo == startRLInfo and middleRLInfo == endRLInfo);
    }

  } else if (shape ==  facet.PARA) {
    selectFitless = (startRLInfo == middleRLInfo) or (middleRLInfo == endRLInfo);

  } else {
    selectFitless = false;

  }

  return selectFitless ? 3 : NOT_A_CELL;
}
