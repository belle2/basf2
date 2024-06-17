/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/variables/BeamBackground.h>

#include <analysis/VariableManager/Manager.h>
#include <framework/dataobjects/BackgroundInfo.h>
#include <framework/dataobjects/BackgroundMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>

#include <algorithm>
#include <limits>

namespace Belle2::Variable {

  namespace {
    const BackgroundInfo::BackgroundDescr* getBackgroundDescription()
    {
      StoreObjPtr<BackgroundInfo> bkgInfo{"", DataStore::c_Persistent};
      // Sanity check
      if (bkgInfo->getMethod() != BackgroundInfo::c_Overlay)
        return nullptr;
      const auto bkgDescriptions = bkgInfo->getBackgrounds();
      // In mdsts we are supposed to have only the "default" type.
      // Also: we should have only one type, but let's be safe here.
      auto bkgDescription = std::find_if(bkgDescriptions.begin(), bkgDescriptions.end(), [](const auto & d) { return d.type == BackgroundMetaData::getDefaultBackgroundOverlayType(); });
      if (bkgDescription != bkgDescriptions.end())
        return &(*bkgDescription);
      else
        return nullptr;
    }
  }

  int beamBackgroundReuseRate(const Particle*)
  {
    const auto* bkgDescription = getBackgroundDescription();
    if (not bkgDescription)
      return std::numeric_limits<int>::quiet_NaN();
    return bkgDescription->reused;
  }

  int beamBackgroundEvents(const Particle*)
  {
    const auto* bkgDescription = getBackgroundDescription();
    if (not bkgDescription)
      return std::numeric_limits<int>::quiet_NaN();
    return bkgDescription->numEvents;
  }

  VARIABLE_GROUP("BeamBackgroundOverlay");
  REGISTER_VARIABLE("beamBackgroundReuseRate", beamBackgroundReuseRate,
                    "[Eventbased] Reuse rate of the background overlay events used for producing the file.", "");
  REGISTER_VARIABLE("beamBackgroundEvents", beamBackgroundEvents,
                    "[Eventbased] Total number of the background overlay events used for producing the file.", "");
}
