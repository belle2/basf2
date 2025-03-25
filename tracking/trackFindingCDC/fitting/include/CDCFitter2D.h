/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/fitting/EFitVariance.h>

#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
    class CDCWireHit;
    class CDCWire;
    class CDCTrajectory2D;
    class CDCSegment3D;
    class CDCAxialSegmentPair;
    class CDCSegment2D;
    class CDCWireHitSegment;
  }
  namespace TrackFindingCDC {
    class CDCObservations2D;

    /// Class implementing a fit for two dimensional trajectory circle using a generic fitting backend.
    template<class AFitMethod>
    class CDCFitter2D : public AFitMethod {

    public:
      /// Default constructor
      CDCFitter2D();

      /// Default destructor
      ~CDCFitter2D();

      /// Fits a collection of observation drift circles.
      TrackingUtilities::CDCTrajectory2D fit(const CDCObservations2D& observations2D) const;

      /// Fits a collection of observation drift circles.
      TrackingUtilities::CDCTrajectory2D fit(CDCObservations2D&& observations2D) const;

      /// Update the trajectory with a fit to the observations.
      void update(TrackingUtilities::CDCTrajectory2D& trajectory2D, const CDCObservations2D& observations2D) const;

      /// Update the trajectory with a fit to the observations.
      void update(TrackingUtilities::CDCTrajectory2D& trajectory2D, CDCObservations2D&& observations2D) const;

      /// Fits the track
      TrackingUtilities::CDCTrajectory2D fit(const TrackingUtilities::CDCTrack& track) const;

      /// Fits the segment
      TrackingUtilities::CDCTrajectory2D fit(const TrackingUtilities::CDCSegment3D& segment) const;

      /// Fits to the two segments
      TrackingUtilities::CDCTrajectory2D fit(const TrackingUtilities::CDCSegment2D& fromSegment,
                                             const TrackingUtilities::CDCSegment2D& toSegment) const;

      /// Fits the segment
      TrackingUtilities::CDCTrajectory2D fit(const TrackingUtilities::CDCSegment2D& segment) const;

      /// Fits to the wire hit observations.
      TrackingUtilities::CDCTrajectory2D fit(const std::vector<const TrackingUtilities::CDCWireHit*>& wireHit) const;

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      TrackingUtilities::CDCTrajectory2D fit(const std::vector<const TrackingUtilities::CDCWire*>& wires) const;

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      TrackingUtilities::CDCTrajectory2D fit(const TrackingUtilities::CDCWireHitSegment& wireHits) const;

      /// Updates the given trajectory inplace from the given segment
      void update(TrackingUtilities::CDCTrajectory2D& trajectory2D, const TrackingUtilities::CDCSegment2D& segment) const;

      /// Updates the given trajectory inplace from the given segment pair
      void update(TrackingUtilities::CDCTrajectory2D& trajectory2D, const TrackingUtilities::CDCAxialSegmentPair& axialSegmentPair) const;

    private:
      /// Fits a collection of hit typs which are convertible to observation circles.
      template<class AHits>
      TrackingUtilities::CDCTrajectory2D fitGeneric(const AHits& hits) const;

      /// Fits together two collections of hit types which are convertible to observation circles.
      template<class AStartHits, class AEndHits>
      TrackingUtilities::CDCTrajectory2D fitGeneric(const AStartHits& startHits, const AEndHits& endHits) const;

      /**
       *  Updates a given trajectory with a fit to a collection of hits types,
       * which are convertible to observation circles.
       */
      template <class AHits>
      void updateGeneric(TrackingUtilities::CDCTrajectory2D& trajectory2D, const AHits& hits) const;

      /**
       *  Updates a given trajectory with a fit to two collection of hit types,
       *  which are convertible to observation circles.
       */
      template <class AStartHits, class AEndHits>
      void updateGeneric(TrackingUtilities::CDCTrajectory2D& trajectory2D,
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
