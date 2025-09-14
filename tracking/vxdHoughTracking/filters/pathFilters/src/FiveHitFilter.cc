/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/FiveHitFilter.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackingUtilities;
using namespace vxdHoughTracking;

void FiveHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "helixFitPocaDCut"), m_helixFitPocaDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a .",
                                m_helixFitPocaDCut);
}

TrackingUtilities::Weight
FiveHitFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackingUtilities::WithWeight<const VXDHoughState*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 4) {
    return NAN;
  }

  return 1.0;
}
