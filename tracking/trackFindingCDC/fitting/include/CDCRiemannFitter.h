/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

      /// Static getter for a given Riemann fitter
      static const CDCRiemannFitter& getFitter(bool fromOrigin, bool line);

      /// Static getter for a line fitter
      static const CDCRiemannFitter& getLineFitter();

      /// Static getter for an origin circle fitter
      static const CDCRiemannFitter& getOriginCircleFitter();
    };
  }
}
