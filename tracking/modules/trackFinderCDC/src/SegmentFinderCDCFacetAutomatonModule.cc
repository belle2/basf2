/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCFacetAutomaton);

SegmentFinderCDCFacetAutomatonModule::SegmentFinderCDCFacetAutomatonModule()
{
  this->setDescription("Generates segments from hits using a cellular automaton build from hit triples (facets).");

  ModuleParamList moduleParamList = this->getParamList();
  m_segmentFinderFacetAutomaton.exposeParameters(&moduleParamList);
  m_segmentSwapper.exposeParameters(&moduleParamList);
  moduleParamList.getParameter<bool>("WriteSegments").setDefaultValue(true);
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("symmetric");
  this->setParamList(moduleParamList);
}

void SegmentFinderCDCFacetAutomatonModule::initialize()
{
  Super::initialize();
  m_segmentFinderFacetAutomaton.initialize();
  m_segmentSwapper.initialize();
}

void SegmentFinderCDCFacetAutomatonModule::beginRun()
{
  Super::beginRun();
  m_segmentFinderFacetAutomaton.beginRun();
  m_segmentSwapper.beginRun();
}

void SegmentFinderCDCFacetAutomatonModule::event()
{
  m_segmentFinderFacetAutomaton.beginEvent();
  m_segmentSwapper.beginEvent();
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  std::vector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();
  m_segments.clear();
  m_segmentFinderFacetAutomaton.apply(wireHits, m_segments);
  m_segmentSwapper.apply(m_segments);
}

void SegmentFinderCDCFacetAutomatonModule::endRun()
{
  m_segmentSwapper.endRun();
  m_segmentFinderFacetAutomaton.endRun();
  Super::endRun();
}

void SegmentFinderCDCFacetAutomatonModule::terminate()
{
  m_segmentSwapper.terminate();
  m_segmentFinderFacetAutomaton.terminate();
  Super::terminate();
}
