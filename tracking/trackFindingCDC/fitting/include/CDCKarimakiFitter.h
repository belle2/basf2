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
    // Guard to prevent repeated instantiations
    extern template class CDCFitter2D<KarimakisMethod>;

    /// Class implementing the fitter using Karimakis method.
    class CDCKarimakiFitter : public CDCFitter2D<KarimakisMethod> {

    public:
      /// Default constructor
      CDCKarimakiFitter();

      /// Default destructor
      ~CDCKarimakiFitter();

      /// Static getter for a general fitter instance with Karimakis method.
      static const CDCKarimakiFitter& getFitter();

      /// Static getter for a line fitter
      static const CDCKarimakiFitter& getLineFitter();

      /// Static getter for a general fitter that does not use the drift length variances
      static const CDCKarimakiFitter& getNoDriftVarianceFitter();
    };
  }
}
