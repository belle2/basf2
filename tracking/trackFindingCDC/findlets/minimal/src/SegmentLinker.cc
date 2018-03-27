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

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

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

  moduleParamList->addParameter(prefixed(prefix, "dealiasLinked"),
                                m_param_dealiasLinked,
                                "Block hits that appear in linked segments such that unlinked reverse and aliases are excluded.",
                                m_param_dealiasLinked);

  moduleParamList->addParameter(prefixed(prefix, "onlyLinked"),
                                m_param_onlyLinked,
                                "Switch to construct only segments that have a linked partner.",
                                m_param_onlyLinked);
}

void SegmentLinker::apply(const std::vector<CDCSegment2D>& inputSegment2Ds,
                          std::vector<CDCSegment2D>& outputSegment2Ds)
{
  if (m_param_wholeSuperLayer) {
    for (const CDCSegment2D& segment2D : inputSegment2Ds) {
      segment2D.setISuperCluster(-1);
    }
  }

  // Obtain the segments as pointers
  std::vector<const CDCSegment2D*> segment2DPtrs = as_pointers<const CDCSegment2D>(inputSegment2Ds);

  // Create linking relations
  m_segment2DRelations.clear();
  m_segment2DRelationCreator.apply(segment2DPtrs, m_segment2DRelations);

  // Find linking paths
  m_segment2DPaths.clear();
  m_cellularPathFinder.apply(segment2DPtrs, m_segment2DRelations, m_segment2DPaths);

  // Unmasked hits in case the blocking logic is requested
  const bool toHits = true;
  for (const CDCSegment2D& segment : inputSegment2Ds) {
    segment.unsetAndForwardMaskedFlag(toHits);
  }

  // Put the linked segments together
  std::vector<CDCSegment2D> tempOutputSegment2Ds;
  tempOutputSegment2Ds.reserve(m_segment2DPaths.size());
  for (const Path<const CDCSegment2D>& segment2DPath : m_segment2DPaths) {
    if (m_param_onlyLinked and segment2DPath.size() == 1) continue;

    // Do not use the single segments blocked in the dealiasing
    if (m_param_dealiasLinked and segment2DPath.size() == 1 and
        (*segment2DPath[0])->hasMaskedFlag()) {
      continue;
    }

    tempOutputSegment2Ds.push_back(CDCSegment2D::condense(segment2DPath));

    // If two segments are linked odds are that it is the correct alias
    // Block the used hits.
    if (m_param_dealiasLinked and segment2DPath.size() > 1) {
      tempOutputSegment2Ds.back().setAndForwardMaskedFlag(toHits);
      for (const CDCSegment2D& otherSegment : inputSegment2Ds) {
        otherSegment.receiveMaskedFlag(toHits);
      }
    }

  }
  outputSegment2Ds = std::move(tempOutputSegment2Ds);
}
