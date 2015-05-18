/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/MCFacetNeighborChooser.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void MCFacetNeighborChooser::clear()
{
  m_mcFacetFilter.clear();
}



void MCFacetNeighborChooser::initialize()
{
  m_mcFacetFilter.initialize();
}



void MCFacetNeighborChooser::terminate()
{
  m_mcFacetFilter.terminate();
}



void MCFacetNeighborChooser::setParameter(const std::string& key, const std::string& value)
{
  if (key == "symmetric") {
    if (value == "true") {
      m_mcFacetFilter.setAllowReverse(true);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else if (value == "false") {
      m_mcFacetFilter.setAllowReverse(false);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else {
      Super::setParameter(key, value);
    }
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> MCFacetNeighborChooser::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the facet relation if the reverse facet relation is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCFacetNeighborChooser::needsTruthInformation()
{
  return true;
}


NeighborWeight MCFacetNeighborChooser::operator()(const CDCFacet& fromFacet,
                                                  const CDCFacet& toFacet)
{
  // the last wire of the neighbor should not be the same as the start wire of the facet
  if (fromFacet.getStartWire() == toFacet.getEndWire()) {
    return NOT_A_NEIGHBOR;
  }
  // Despite of that two facets are neighbors if both are true facets
  // That also implies the correct tof alignment of the hits not common to both facets
  CellWeight fromFacetWeight = m_mcFacetFilter(fromFacet);
  CellWeight toFacetWeight = m_mcFacetFilter(toFacet);

  bool mcDecision = (not isNotACell(fromFacetWeight)) and (not isNotACell(toFacetWeight));

  // the weight must be -2 because the overlap of the facets is two points
  // so the amount of two facets is 4 points hence the cellular automat
  // must calculate 3 + (-2) + 3 = 4 as cellstate
  // this can of course be adjusted for a more realistic information measure
  // ( together with the facet creator filter)
  return mcDecision ? -2.0 : NOT_A_NEIGHBOR;
}
