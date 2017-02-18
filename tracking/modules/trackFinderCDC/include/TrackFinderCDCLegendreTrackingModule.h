/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>
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
    class TrackFinderCDCLegendreTrackingModule : public FindletModule<AxialTrackFinderLegendre> {

      /// The base class
      using Super = FindletModule<AxialTrackFinderLegendre>;

    public:
      /// Constructor
      TrackFinderCDCLegendreTrackingModule();
    };
  }
}
