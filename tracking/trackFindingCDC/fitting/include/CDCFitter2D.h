/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/fitting/EFitVariance.h>
#include <tracking/trackFindingCDC/fitting/EFitPos.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment3D;
    class CDCAxialSegmentPair;
    class CDCSegment2D;
    class CDCWireHitSegment;
    class CDCWireHit;
    class CDCTrajectory2D;
    class CDCObservations2D;

    class CDCWire;

    /// Class implementing a fit for two dimensional trajectory circle using a generic fitting backend.
    template<class AFitMethod>
    class CDCFitter2D : public AFitMethod {

    public:
      /// Default constructor
      CDCFitter2D();

      /// Default destructor
      ~CDCFitter2D();

      /// Fits a collection of observation drift circles.
      CDCTrajectory2D fit(const CDCObservations2D& observations2D) const;

      /// Fits a collection of observation drift circles.
      CDCTrajectory2D fit(CDCObservations2D&& observations2D) const;

      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectory2D& trajectory2D, const CDCObservations2D& observations2D) const;

      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectory2D& trajectory2D, CDCObservations2D&& observations2D) const;

      /// Fits the track
      CDCTrajectory2D fit(const CDCTrack& track) const;

      /// Fits the segment
      CDCTrajectory2D fit(const CDCSegment3D& segment) const;

      /// Fits to the two segments
      CDCTrajectory2D fit(const CDCSegment2D& fromSegment, const CDCSegment2D& toSegment) const;

      /// Fits the segment
      CDCTrajectory2D fit(const CDCSegment2D& segment) const;

      /// Fits to the wire hit observations.
      CDCTrajectory2D fit(const std::vector<const CDCWireHit*>& wireHit) const;

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      CDCTrajectory2D fit(const std::vector<const CDCWire*>& wires) const;

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      CDCTrajectory2D fit(const CDCWireHitSegment& wireHits) const;

      /// Updates the given trajectory inplace from the given segment
      void update(CDCTrajectory2D& trajectory2D, const CDCSegment2D& segment) const;

      /// Updates the given trajectory inplace from the given segment pair
      void update(CDCTrajectory2D& trajectory2D, const CDCAxialSegmentPair& axialSegmentPair) const;

    private:
      /// Fits a collection of hit typs which are convertable to observation circles.
      template<class AHits>
      CDCTrajectory2D fitGeneric(const AHits& hits) const;

      /// Fits together two collections of hit types which are convertable to observation circles.
      template<class AStartHits, class AEndHits>
      CDCTrajectory2D fitGeneric(const AStartHits& startHits, const AEndHits& endHits) const;

      /**
       *  Updates a given trajectory with a fit to a collection of hits types,
       * which are convertable to observation circles.
       */
      template <class AHits>
      void updateGeneric(CDCTrajectory2D& trajectory2D, const AHits& hits) const;

      /**
       *  Updates a given trajectory with a fit to two collection of hit types,
       *  which are convertable to observation circles.
       */
      template <class AStartHits, class AEndHits>
      void updateGeneric(CDCTrajectory2D& trajectory2D,
                         const AStartHits& startHits,
                         const AEndHits& endHits) const;

    public:
      //set which information should be used from the recohits
      //useOnlyPosition is standard

      /// Setup the fitter to use only the reconstructed positions of the hits
      void useOnlyPosition();

      /// Setup the fitter to use only reference position and the drift length with right left orientation
      void useOnlyOrientation();

      /// Setup the fitter to use both the reconstructed position and the reference position and the drift length with right left orientation.
      void usePositionAndOrientation();

      /// Setup the fitter to use the given variance measure by default.
      void setFitVariance(EFitVariance fitVariance);

    private:
      /// Flag indicating the reconstructed position shall be used in the fit.
      bool m_usePosition = true;

      /// Flag indicating the reference position and drift length with right left orientation shall be used in the fit.
      bool m_useOrientation = false;

      /// Default variance to be used in the fit.
      EFitVariance m_fitVariance = EFitVariance::c_Proper;
    };
  }
}
