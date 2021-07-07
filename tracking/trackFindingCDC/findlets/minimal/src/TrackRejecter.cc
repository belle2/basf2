/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackRejecter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::Chooseable<BaseTrackFilter>;
template class TrackFindingCDC::ChooseableFilter<TrackFilterFactory>;

TrackRejecter::TrackRejecter(const std::string& defaultFilterName)
  : m_trackFilter(defaultFilterName)
{
  this->addProcessingSignalListener(&m_trackFilter);
}

std::string TrackRejecter::getDescription()
{
  return "Deletes fake tracks that have been rejected by a filter";
}

void TrackRejecter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackFilter.exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "deleteRejected"),
                                m_param_deleteRejected,
                                "Delete the rejected tracks instead of marking them as background.",
                                m_param_deleteRejected);
}

void TrackRejecter::apply(std::vector<CDCTrack>& tracks)
{
  auto reject = [this](CDCTrack & track) {
    double filterWeight = m_trackFilter(track);
    track->setCellWeight(filterWeight);
    if (std::isnan(filterWeight)) {
      track->setBackgroundFlag();
      track->setTakenFlag();
      return true;
    } else {
      return false;
    }
  };

  if (m_param_deleteRejected) {
    erase_remove_if(tracks, reject);
  } else {
    std::for_each(tracks.begin(), tracks.end(), reject);
  }
}
