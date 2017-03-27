/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentLinker.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <vector>
#include <string>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentLinker::SegmentLinker()
{
  this->addProcessingSignalListener(&m_segment2DRelationCreator);
}

std::string SegmentLinker::getDescription()
{
  return "Links segments by extraction of segment paths in a cellular automaton.";
}

void SegmentLinker::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_segment2DRelationCreator.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "wholeSuperLayer"),
                                m_param_wholeSuperLayer,
                                "Link segment in the whole superlayer instead of inside the super cluster.",
                                m_param_wholeSuperLayer);
}

void SegmentLinker::apply(const std::vector<CDCSegment2D>& inputSegment2Ds,
                          std::vector<CDCSegment2D>& outputSegment2Ds)
{
  if (m_param_wholeSuperLayer) {
    for (const CDCSegment2D& segment2D : inputSegment2Ds) {
      segment2D.setISuperCluster(-1);
    }
  }

  // Create linking relations
  m_segment2DRelations.clear();
  m_segment2DRelationCreator.apply(inputSegment2Ds, m_segment2DRelations);

  // Find linking paths
  m_segment2DPaths.clear();
  WeightedNeighborhood<const CDCSegment2D> segment2DNeighborhood(m_segment2DRelations);
  m_cellularPathFinder.apply(inputSegment2Ds, segment2DNeighborhood, m_segment2DPaths);

  // Put the linked segments together
  std::vector<CDCSegment2D> tempOutputSegment2Ds;
  tempOutputSegment2Ds.reserve(m_segment2DPaths.size());
  for (const Path<const CDCSegment2D>& segment2DPath : m_segment2DPaths) {
    tempOutputSegment2Ds.push_back(CDCSegment2D::condense(segment2DPath));
  }
  outputSegment2Ds = std::move(tempOutputSegment2Ds);
}
