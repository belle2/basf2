/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/UnionRecordingFacetFilter.h>

#include <tracking/trackFindingCDC/filters/facet/FitFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/BendFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/FitlessFacetVarSet.h>
#include <tracking/trackFindingCDC/filters/facet/BasicFacetVarSet.h>

#include <tracking/trackFindingCDC/filters/facet/MVAFacetFilter.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UnionRecordingFilter<FacetFilterFactory>;

std::vector<std::string> UnionRecordingFacetFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "fitless", "bend", "fit", "mva"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<const CDCFacet> >
UnionRecordingFacetFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return std::make_unique<BasicFacetVarSet>();
  } else if (name == "fitless") {
    return std::make_unique<FitlessFacetVarSet>();
  } else if (name == "bend") {
    return std::make_unique<BendFacetVarSet>();
  } else if (name == "fit") {
    return std::make_unique<FitFacetVarSet>();
  } else if (name == "mva") {
    MVAFacetFilter mvaFacetFilter;
    return std::move(mvaFacetFilter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
