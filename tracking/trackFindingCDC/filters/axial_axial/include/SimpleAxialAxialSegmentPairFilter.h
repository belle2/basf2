/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEAXIALAXIALSEGMENTPAIRFILTER_H_
#define SIMPLEAXIALAXIALSEGMENTPAIRFILTER_H_

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialAxialSegmentPair.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleAxialAxialSegmentPairFilter {

    public:

      /// Constructor
      SimpleAxialAxialSegmentPairFilter();

      /// Empty destructor
      ~SimpleAxialAxialSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair);

      /// Returns the trajectory of the axial segment. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const;

      /// Returns the trajectory of the axial to axial segment pair. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialAxialSegmentPair& axialAxialSegmentPair) const;

      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }

    private:
      CDCRiemannFitter m_riemannFitter; ///< Memory of the Riemann fitter for the circle fits.

    }; // end class SimpleAxialAxialSegmentPairFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SIMPLEAXIALAXIALSEGMENTPAIRFILTER_H_
