/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRIEMANNFITTER_H
#define CDCRIEMANNFITTER_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include "CDCObservations2D.h"
#include "CDCFitter2D.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class CDCRiemannFitter : public CDCFitter2D<Belle2::CDCLocalTracking::ExtendedRiemannsMethod> {

    public:
      ///Static getter for a general Riemann fitter
      static const CDCRiemannFitter& getFitter();

      ///Static getter for a line fitter
      static const CDCRiemannFitter& getLineFitter();

      ///Static getter for an origin circle fitter
      static const CDCRiemannFitter& getOriginCircleFitter();

    public:
      ///Empty constructor
      CDCRiemannFitter();

      ///Empty destructor
      ~CDCRiemannFitter();

    public:
      /** ROOT Macro to make CDCRiemannFitter a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCRiemannFitter, 1);
    }; //class

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRIEMANNFITTER
