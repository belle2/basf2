/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/DATCONFindlet.h>
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.icc.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONFindlet::~DATCONFindlet() = default;

DATCONFindlet::DATCONFindlet()
{
  addProcessingSignalListener(&m_spacePointLoaderAndPreparer);
  addProcessingSignalListener(&m_interceptFinder);
  addProcessingSignalListener(&m_interceptFinderSimple);
  addProcessingSignalListener(&m_rawTCCleaner);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_recoTrackStorer);
  addProcessingSignalListener(&m_roiFinder);
}

void DATCONFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_spacePointLoaderAndPreparer.exposeParameters(moduleParamList, prefix);
  m_interceptFinder.exposeParameters(moduleParamList, prefix);
  m_interceptFinderSimple.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "simple"));
  m_rawTCCleaner.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "finalOverlapResolver"));
  m_recoTrackStorer.exposeParameters(moduleParamList, prefix);
  m_roiFinder.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>("relationFilter").setDefaultValue("angleAndTime");
  moduleParamList->getParameter<std::string>("twoHitFilter").setDefaultValue("twoHitVirtualIPQI");
  moduleParamList->getParameter<std::string>("threeHitFilter").setDefaultValue("qualityIndicator");
  moduleParamList->getParameter<std::string>("fourHitFilter").setDefaultValue("qualityIndicator");
  moduleParamList->getParameter<std::string>("fiveHitFilter").setDefaultValue("qualityIndicator");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useSubHoughSpaces"), m_param_useSubHoughSpaces,
                                "Use Hough spaces working on a subset of hits (=true), or just one Hough space working on all hits at the same time (=false)?",
                                m_param_useSubHoughSpaces);
}

void DATCONFindlet::beginEvent()
{
  Super::beginEvent();

  m_spacePointVector.clear();
  m_hitDataVector.clear();
  m_rawTrackCandidates.clear();
  m_trackCandidates.clear();
}

void DATCONFindlet::apply()
{
  m_spacePointLoaderAndPreparer.apply(m_spacePointVector, m_hitDataVector);
  B2DEBUG(29, "m_hitDataVector.size(): " << m_hitDataVector.size());

  if (m_param_useSubHoughSpaces) {
    m_interceptFinder.apply(m_hitDataVector, m_rawTrackCandidates);
  } else {
    m_interceptFinderSimple.apply(m_hitDataVector, m_rawTrackCandidates);
  }
  B2DEBUG(29, "m_rawTrackCandidates.size: " << m_rawTrackCandidates.size());

  m_rawTCCleaner.apply(m_rawTrackCandidates, m_trackCandidates);

  // sort by number of hits in the track candidate and by the QI
  std::sort(m_trackCandidates.begin(), m_trackCandidates.end(),
  [](const SpacePointTrackCand & a, const SpacePointTrackCand & b) {
    return ((a.getNHits() > b.getNHits()) or
            (a.getNHits() == b.getNHits() and a.getQualityIndicator() > b.getQualityIndicator()));
  });

  m_overlapResolver.apply(m_trackCandidates);

  m_recoTrackStorer.apply(m_trackCandidates, m_spacePointVector);

  // A check if or if not ROIs shall be calculated is performed within the m_roiFinder findlet to
  // avoid creation and registration of the ROIs and PXDIntercept StoreArrays
  // such that StoreArrays with the same name can be registered elsewhere.
  m_roiFinder.apply(m_trackCandidates);
}
