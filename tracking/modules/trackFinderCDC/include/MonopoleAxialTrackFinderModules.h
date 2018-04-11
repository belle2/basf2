/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost, Dmitrii Neverov                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/MonopoleAxialTrackFinderLegendre.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * CDC tracking module, using Legendre transformation of the drift time circles.
     * This is a module, performing tracking in the CDC. It is based on the paper
     * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
     * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
     */
    class TFCDC_MonopoleAxialTrackFinderLegendreModule : public FindletModule<MonopoleAxialTrackFinderLegendre> {

      /// Type of the base class
      using Super = FindletModule<MonopoleAxialTrackFinderLegendre>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_MonopoleAxialTrackFinderLegendreModule();
    };
  }
}
