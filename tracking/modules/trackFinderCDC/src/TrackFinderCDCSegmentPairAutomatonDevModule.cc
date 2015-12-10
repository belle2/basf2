/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCSegmentPairAutomatonDevModule.h>

#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairFilters.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilters.h>
#include <tracking/trackFindingCDC/filters/trackRelation/TrackRelationFilters.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCSegmentPairAutomatonDev);

TrackFinderCDCSegmentPairAutomatonDevModule::TrackFinderCDCSegmentPairAutomatonDevModule() :
  m_segmentPairFilterFactory("simple"),
  m_segmentPairRelationFilterFactory("simple"),
  m_trackRelationFilterFactory("none")
{
  setDescription("Versatile module with adjustable filters for track generation.");

  ModuleParamList moduleParamList = this->getParamList();

  m_segmentPairFilterFactory.exposeParameters(&moduleParamList);
  m_segmentPairRelationFilterFactory.exposeParameters(&moduleParamList);
  m_trackRelationFilterFactory.exposeParameters(&moduleParamList);

  this->setParamList(moduleParamList);
}

void TrackFinderCDCSegmentPairAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentPairFilter> ptrSegmentPairFilter = m_segmentPairFilterFactory.create();
  setSegmentPairFilter(std::move(ptrSegmentPairFilter));

  std::unique_ptr<BaseSegmentPairRelationFilter>
  ptrSegmentPairRelationFilter = m_segmentPairRelationFilterFactory.create();
  setSegmentPairRelationFilter(std::move(ptrSegmentPairRelationFilter));

  std::unique_ptr<BaseTrackRelationFilter>
  ptrTrackRelationFilter = m_trackRelationFilterFactory.create();
  setTrackRelationFilter(std::move(ptrTrackRelationFilter));

  TrackFinderCDCSegmentPairAutomatonImplModule<>::initialize();

  if (getSegmentPairFilter()->needsTruthInformation() or
      getSegmentPairRelationFilter()->needsTruthInformation() or
      getTrackRelationFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}

void TrackFinderCDCSegmentPairAutomatonDevModule::event()
{
  if (getSegmentPairFilter()->needsTruthInformation() or
      getSegmentPairRelationFilter()->needsTruthInformation() or
      getTrackRelationFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  TrackFinderCDCSegmentPairAutomatonImplModule<>::event();
}
