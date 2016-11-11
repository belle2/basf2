/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentExporter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <genfit/TrackCand.h>
#include <framework/datastore/StoreArray.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string SegmentExporter::getDescription()
{
  return "Creates reconstruction track candidates from each individual segments.";
}

void SegmentExporter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "exportSegments"),
                                m_param_exportSegments,
                                "Switch for the creation of track candidates for each segment.",
                                m_param_exportSegments);

  moduleParamList->addParameter(prefixed(prefix, "exportSegmentsInto"),
                                m_param_exportSegmentsInto,
                                "Name of the output StoreArray of track candidates.",
                                m_param_exportSegmentsInto);
}

void SegmentExporter::initialize()
{
  // Output StoreArray
  if (m_param_exportSegments) {
    StoreArray <genfit::TrackCand> storedGFTrackCands(m_param_exportSegmentsInto);
    storedGFTrackCands.registerInDataStore();
  }
  Super::initialize();
}

void SegmentExporter::apply(const std::vector<CDCRecoSegment2D>& segments)
{
  // Put code to generate gf track cands here if requested.
  if (m_param_exportSegments) {
    StoreArray<genfit::TrackCand> storedGFTrackCands(m_param_exportSegmentsInto);
    for (const CDCRecoSegment2D& segment : segments) {
      genfit::TrackCand* ptrTrackCand = storedGFTrackCands.appendNew();
      segment.fillInto(*ptrTrackCand);
    }
  }
}
