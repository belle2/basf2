/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/fitting/CDCFitter2D.h>
#include <tracking/trackFindingCDC/fitting/KarimakisMethod.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class implementing the fitter using Karimakis method.
    class CDCKarimakiFitter : public CDCFitter2D<Belle2::TrackFindingCDC::KarimakisMethod> {

    public:
      /// Static getter for a general fitter instance with Karimakis method.
      static const CDCKarimakiFitter& getFitter();

      /// Static getter for a line fitter
      static const CDCKarimakiFitter& getLineFitter();

      /// Static getter for a general fitter that does not use the drift length variances
      static const CDCKarimakiFitter& getNoDriftVarianceFitter();
    };
  }
}
