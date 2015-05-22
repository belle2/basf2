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

#include <tracking/trackFindingCDC/filters/segment_pair/SegmentPairFilters.h>
#include <tracking/trackFindingCDC/filters/segment_pair_relation/SegmentPairRelationFilters.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCSegmentPairAutomatonDev);

TrackFinderCDCSegmentPairAutomatonDevModule::TrackFinderCDCSegmentPairAutomatonDevModule() :
  TrackFinderCDCSegmentPairAutomatonImplModule<>(c_None),
  m_segmentPairFilterFactory("simple"),
  m_segmentPairRelationFilterFactory("simple")
{
  setDescription("Versatile module with adjustable filters for track generation.");

  m_segmentPairFilterFactory.exposeParameters(this);
  m_segmentPairRelationFilterFactory.exposeParameters(this);
}

void TrackFinderCDCSegmentPairAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentPairFilter> ptrSegmentPairFilter = m_segmentPairFilterFactory.create();
  setSegmentPairFilter(std::move(ptrSegmentPairFilter));

  std::unique_ptr<BaseSegmentPairRelationFilter>
  ptrSegmentPairRelationFilter = m_segmentPairRelationFilterFactory.create();
  setSegmentPairRelationFilter(std::move(ptrSegmentPairRelationFilter));

  TrackFinderCDCSegmentPairAutomatonImplModule<>::initialize();

  if (getSegmentPairFilter()->needsTruthInformation() or
      getSegmentPairRelationFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}

void TrackFinderCDCSegmentPairAutomatonDevModule::event()
{
  if (getSegmentPairFilter()->needsTruthInformation() or
      getSegmentPairRelationFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  TrackFinderCDCSegmentPairAutomatonImplModule<>::event();
}
