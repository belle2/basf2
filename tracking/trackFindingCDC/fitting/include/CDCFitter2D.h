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

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegments.h>
#include <tracking/trackFindingCDC/numerics/InfoTypes.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class implementing a fit for two dimensional trajectory circle using a generic fitting backend.
    template<class FitMethod>
    class CDCFitter2D : public FitMethod {

    public:
      /// Empty constructor
      CDCFitter2D() :
        FitMethod(),
        m_usePosition(false),
        m_useOrientation(true)
      {;}

      /// Empty destructor
      ~CDCFitter2D()
      {;}

      /// Fits a collection of observation drift circles.
      CDCTrajectory2D fit(CDCObservations2D observations2D) const
      {
        CDCTrajectory2D result;
        update(result, observations2D);
        return result;
      }

      /// Fits a collection of hit typs which are convertable to observation circles.
      template<class Hits>
      CDCTrajectory2D fit(const Hits& hits) const
      {
        CDCTrajectory2D result;
        update(result, hits);
        return result;
      }

      /// Fits together two collections of hit types which are convertable to observation circles.
      template<class StartHits, class EndHits>
      CDCTrajectory2D fit(const StartHits& startHits, const EndHits& endHits) const
      {
        CDCTrajectory2D result;
        update(result, startHits, endHits);
        return result;
      }

      /// Fits the segment
      CDCTrajectory2D fit(const CDCRecoSegment2D& segment) const
      {
        CDCTrajectory2D result;
        update(result, segment);
        return result;
      }

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      CDCTrajectory2D fit(const std::vector<const Belle2::TrackFindingCDC::CDCWire*>& wires) const
      {
        CDCTrajectory2D result;
        update(result, wires);
        return result;
      }

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      CDCTrajectory2D fit(const CDCWireHitSegment& wireHits) const
      {
        CDCTrajectory2D result;
        update(result, wireHits);
        return result;
      }

      /** Updates a given trajectory with a fit to two collection of hit types,
       * which are convertable to observation circles.
       */
      template<class StartHits, class EndHits>
      void update(CDCTrajectory2D& trajectory2D,
                  const StartHits& startHits,
                  const EndHits& endHits) const
      {
        CDCObservations2D observations2D;
        if (m_usePosition) {
          observations2D.appendRange(startHits, true);
        }
        if (m_useOrientation) {
          observations2D.appendRange(startHits, false);
        }

        if (m_usePosition) {
          observations2D.appendRange(endHits, true);
        }
        if (m_useOrientation) {
          observations2D.appendRange(endHits, false);
        }

        if (observations2D.size() < 4) {
          trajectory2D.clear();
        } else {
          FitMethod::update(trajectory2D, observations2D);
        }
      }

      /** Updates a given trajectory with a fit to a collection of hits types,
       *  which are convertable to observation circles.
       */
      template<class Hits>
      void update(CDCTrajectory2D& trajectory2D, const Hits& hits) const
      {
        CDCObservations2D observations2D;
        if (m_usePosition) {
          observations2D.appendRange(hits, true);
        }
        if (m_useOrientation) {
          observations2D.appendRange(hits, false);
        }

        if (observations2D.size() < 4) {
          trajectory2D.clear();
        } else {
          FitMethod::update(trajectory2D, observations2D);
        }
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
      /// Flag indicating the reconstructed position shall be used in the fit.
      bool m_usePosition;

      /// Flag indicating the reference position and drift length with right left orientation shall be used in the fit.
      bool m_useOrientation;

    }; //class

  } // end namespace TrackFindingCDC
} // namespace Belle2
