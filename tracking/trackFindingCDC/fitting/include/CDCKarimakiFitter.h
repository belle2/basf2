/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
