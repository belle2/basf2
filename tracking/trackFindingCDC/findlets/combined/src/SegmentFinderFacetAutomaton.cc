/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentFinderFacetAutomaton::SegmentFinderFacetAutomaton()
{
  this->addProcessingSignalListener(&m_facetCreator);
  this->addProcessingSignalListener(&m_facetRelationCreator);
  this->addProcessingSignalListener(&m_segmentCreatorFacetAutomaton);
  this->addProcessingSignalListener(&m_segmentLinker);

  this->addProcessingSignalListener(&m_segmentFitter);
  this->addProcessingSignalListener(&m_segmentAliasResolver);
  this->addProcessingSignalListener(&m_segmentOrienter);

  this->addProcessingSignalListener(&m_facetSwapper);

  m_facets.reserve(1000);
  m_facetRelations.reserve(1000);
  m_segments.reserve(200);
  m_intermediateSegments.reserve(200);

  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("curling");
}

std::string SegmentFinderFacetAutomaton::getDescription()
{
  return "Generates segments from hits using a cellular automaton build from hit triples (facets).";
}

void SegmentFinderFacetAutomaton::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_facetCreator.exposeParameters(moduleParamList, prefixed(prefix, "Facet"));
  m_facetRelationCreator.exposeParameters(moduleParamList, prefixed(prefix, "FacetRelation"));
  m_segmentCreatorFacetAutomaton.exposeParameters(moduleParamList, prefix);
  m_segmentLinker.exposeParameters(moduleParamList, prefixed(prefix, "SegmentRelation"));

  m_segmentFitter.exposeParameters(moduleParamList, prefix);
  m_segmentAliasResolver.exposeParameters(moduleParamList, prefix);
  m_segmentOrienter.exposeParameters(moduleParamList, prefix);

  m_facetSwapper.exposeParameters(moduleParamList, prefix);
}

void SegmentFinderFacetAutomaton::beginEvent()
{
  m_facets.clear();
  m_facetRelations.clear();
  m_segments.clear();
  m_intermediateSegments.clear();
  Super::beginEvent();
}

void SegmentFinderFacetAutomaton::apply(std::vector<CDCWireHitCluster>& clusters, std::vector<CDCSegment2D>& outputSegments)
{
  outputSegments.reserve(200);

  m_facetCreator.apply(clusters, m_facets);

  std::vector<const CDCFacet*> facetPtrs = as_pointers<const CDCFacet>(m_facets);
  m_facetRelationCreator.apply(facetPtrs, m_facetRelations);
  if (m_facetRelations.size() == 0) return; // Break point for facet recording runs

  m_segmentCreatorFacetAutomaton.apply(m_facets, m_facetRelations, m_segments);
  m_segmentFitter.apply(m_segments);

  m_segmentOrienter.apply(m_segments, m_intermediateSegments);
  m_segmentFitter.apply(m_intermediateSegments);

  m_segmentAliasResolver.apply(m_intermediateSegments);
  m_segmentFitter.apply(m_intermediateSegments);

  m_segmentLinker.apply(m_intermediateSegments, outputSegments);
  m_segmentFitter.apply(outputSegments);
  m_segmentFitter.apply(outputSegments);

  // Move facets to the DataStore
  m_facetSwapper.apply(m_facets);
}
