/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/MCFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseFacetRelationFilter>;

MCFacetRelationFilter::MCFacetRelationFilter(bool allowReverse)
  : Super(allowReverse)
  , m_mcFacetFilter(allowReverse)
{
  this->addProcessingSignalListener(&m_mcFacetFilter);
}

void MCFacetRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                             const std::string& prefix)
{
  m_mcFacetFilter.exposeParameters(moduleParamList, prefix);
}

void MCFacetRelationFilter::initialize()
{
  Super::initialize();
  setAllowReverse(m_mcFacetFilter.getAllowReverse());
}

Weight MCFacetRelationFilter::operator()(const CDCFacet& fromFacet,
                                         const CDCFacet& toFacet)
{
  // the last wire of the neighbor should not be the same as the start wire of the facet
  if (fromFacet.getStartWireHit().isOnWire(toFacet.getEndWire())) return NAN;

  // Despite of that two facets are neighbors if both are true facets
  // That also implies the correct tof alignment of the hits not common to both facets
  Weight fromFacetWeight = m_mcFacetFilter(fromFacet);
  Weight toFacetWeight = m_mcFacetFilter(toFacet);

  if ((fromFacetWeight > 0) and (toFacetWeight > 0)) {
    return 2;
  }

  if (getAllowReverse() and (fromFacetWeight < 0) and (toFacetWeight < 0)) {
    return -2;
  }

  return NAN;
}
