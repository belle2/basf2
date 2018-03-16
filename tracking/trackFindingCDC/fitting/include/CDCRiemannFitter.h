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

#include <tracking/trackFindingCDC/fitting/ExtendedRiemannsMethod.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class CDCFitter2D<ExtendedRiemannsMethod>;

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class CDCRiemannFitter : public CDCFitter2D<Belle2::TrackFindingCDC::ExtendedRiemannsMethod> {

    public:
      /// Default constructor
      CDCRiemannFitter();

      /// Default destructor
      ~CDCRiemannFitter();

      /// Static getter for a general Riemann fitter
      static const CDCRiemannFitter& getFitter();

      /// Static getter for a line fitter
      static const CDCRiemannFitter& getLineFitter();

      /// Static getter for an origin circle fitter
      static const CDCRiemannFitter& getOriginCircleFitter();
    };
  }
}
