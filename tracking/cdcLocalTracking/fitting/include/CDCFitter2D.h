/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCFITTER2D_H
#define CDCFITTER2D_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>

#include "RiemannsMethod.h"
#include "ExtendedRiemannsMethod.h"
#include "KarimakisMethod.h"

#ifdef __CINT__
//Unpack specialisation from all namespaces to spoon feeding them to ROOT
typedef Belle2::CDCLocalTracking::RiemannsMethod RiemannsMethod;
typedef Belle2::CDCLocalTracking::ExtendedRiemannsMethod ExtendedRiemannsMethod;
typedef Belle2::CDCLocalTracking::KarimakisMethod KarimakisMethod;
#endif // __CINT__

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    template<class FitMethod>
    class CDCFitter2D : public FitMethod {

    public:
      /// Empty constructor
      CDCFitter2D() :
        FitMethod(),
        m_usePosition(false),
        m_useOrientation(true)
      { ; }

      /// Empty destructor
      ~CDCFitter2D()
      { ; }

      /// Fits a collection of hits typs which are convertable to observation circles.
      template<class Hits>
      CDCTrajectory2D fit(const Hits& hits) const {
        CDCTrajectory2D result;
        update(result, hits);
        return result;
      }

      /// Fits the segment
      CDCTrajectory2D fit(const CDCRecoSegment2D& segment) const {
        CDCTrajectory2D result;
        update(result, segment);
        return result;
      }

      /// Updates a given trajectory with a fit to a collection of hits typs, which are convertable to observation circles.
      template<class Hits>
      void update(CDCTrajectory2D& trajectory2D, const Hits& hits) const {
        CDCObservations2D observations2D;
        if (m_usePosition) {
          observations2D.append(hits, true);
        }
        if (m_useOrientation) {
          observations2D.append(hits, false);
        }

        FitMethod::update(trajectory2D, observations2D);

        ForwardBackwardInfo isCoaligned = observations2D.isCoaligned(trajectory2D);
        if (isCoaligned == BACKWARD) trajectory2D.reverse();
        //else if (isCoaligned != FORWARD) B2WARNING("Fit cannot be oriented correctly");
      }

      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
      { FitMethod::update(trajectory2D, observations2D); }


      //set which information should be used from the recohits
      //useOnlyOrientation is standard

      /// Setup the fitter to use only the reconstructed positions of the hits
      void useOnlyPosition() { m_usePosition = true; m_useOrientation = false;}

      /// Setup the fitter to use only reference position and the drift length with right left orientation
      void useOnlyOrientation() { m_usePosition = false; m_useOrientation = true;}

      /// Setup the fitter to use both the reconstructed position and the reference position and the drift length with right left orientation.
      void usePositionAndOrientation() { m_usePosition = true; m_useOrientation = true;}

    private:
      bool m_usePosition; ///< Flag indicating the reconstructed position shall be used in the fit.
      bool m_useOrientation; ///< Flag indicating the reference position and drift length with right left orientation shall be used in the fit.

    public:
      /** ROOT Macro to make CDCFitter2D a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCFitter2D, 1);

    }; //class

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCFITTER2D
