/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentRejecter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::ChooseableFilter<SegmentFilterFactory>;

SegmentRejecter::SegmentRejecter(const std::string& defaultFilterName)
  : m_segmentFilter(defaultFilterName)
{
  this->addProcessingSignalListener(&m_segmentFilter);
}

std::string SegmentRejecter::getDescription()
{
  return "Deletes fake segments that have been rejected by a filter";
}

void SegmentRejecter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_segmentFilter.exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "deleteRejected"),
                                m_param_deleteRejected,
                                "Delete the rejected segments instead of marking this as background.",
                                m_param_deleteRejected);
}

void SegmentRejecter::apply(std::vector<CDCSegment2D>& segment2Ds)
{
  auto reject = [this](CDCSegment2D & segment2D) {
    double filterWeight = m_segmentFilter(segment2D);
    segment2D->setCellWeight(filterWeight);
    if (std::isnan(filterWeight)) {
      segment2D->setBackgroundFlag();
      segment2D->setTakenFlag();
      return true;
    } else {
      return false;
    }
  };

  if (m_param_deleteRejected) {
    erase_remove_if(segment2Ds, reject);
  } else {
    std::for_each(segment2Ds.begin(), segment2Ds.end(), reject);
  }
}
