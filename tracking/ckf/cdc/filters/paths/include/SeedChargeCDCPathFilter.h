/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/cdc/filters/paths/BaseCDCPathFilter.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  /// Check if charge of fitted path corresponds to charge of seed
  class SeedChargeCDCPathFilter : public BaseCDCPathFilter {
  public:
    /// Main function: return the charge of the path
    TrackFindingCDC::Weight operator()(const BaseCDCPathFilter::Object& path) final {
      // Not able to find any hits
      if (path.size() == 1)
      {
        return NAN;
      }

      // Filter does not make sense if seeds initialited with 0 charge
      if (path.front().getSeed()->getChargeSeed() == 0)
      {
        return 0.;
      }

      // We don't want tracks that flipped charge
      if (path.front().getSeed()->getChargeSeed() != path.back().getTrackState().getCharge())
      {
        B2DEBUG(29, "Flipped charge: " << path.front().getSeed()->getChargeSeed() << "->" << path.back().getTrackState().getCharge());
        return NAN;
      }

      return path.front().getSeed()->getChargeSeed();
    }
  };
}
