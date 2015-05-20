/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_triple_relation/MCSegmentTripleRelationFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentTripleRelationFilter::MCSegmentTripleRelationFilter(bool allowReverse) :
  m_mcSegmentTripleFilter(allowReverse)
{
}

void MCSegmentTripleRelationFilter::clear()
{
  m_mcSegmentTripleFilter.clear();
}

void MCSegmentTripleRelationFilter::initialize()
{
  m_mcSegmentTripleFilter.initialize();
}

void MCSegmentTripleRelationFilter::terminate()
{
  m_mcSegmentTripleFilter.terminate();
}

void MCSegmentTripleRelationFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "symmetric") {
    if (value == "true") {
      setAllowReverse(true);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else if (value == "false") {
      setAllowReverse(false);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else {
      Super::setParameter(key, value);
    }
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> MCSegmentTripleRelationFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the facet relation if the reverse segment triple relation is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCSegmentTripleRelationFilter::needsTruthInformation()
{
  return true;
}

NeighborWeight MCSegmentTripleRelationFilter::operator()(const CDCSegmentTriple& triple,
                                                         const CDCSegmentTriple& neighborTriple)
{
  CellWeight mcTripleWeight = m_mcSegmentTripleFilter(triple);
  CellWeight mcNeighborTripleWeight = m_mcSegmentTripleFilter(neighborTriple);

  bool mcDecision = (not isNotACell(mcTripleWeight)) and (not isNotACell(mcNeighborTripleWeight));
  return mcDecision ? - neighborTriple.getStart()->size() : NOT_A_NEIGHBOR;
}
