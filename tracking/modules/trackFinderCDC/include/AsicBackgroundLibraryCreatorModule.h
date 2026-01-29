/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/complete/AsicBackgroundLibraryCreator.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>
#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Module for creating ASIC background library
    class AsicBackgroundLibraryCreatorModule : public TrackingUtilities::FindletModule<AsicBackgroundLibraryCreator> {
    };
  }
}
