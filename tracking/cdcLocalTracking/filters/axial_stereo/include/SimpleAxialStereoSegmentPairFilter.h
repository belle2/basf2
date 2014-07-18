/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEAXIALSTEREOSEGMENTPAIRFILTER_H
#define SIMPLEAXIALSTEREOSEGMENTPAIRFILTER_H

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialStereoSegmentPair.h>
#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleAxialStereoSegmentPairFilter {

    public:

      /// Constructor
      SimpleAxialStereoSegmentPairFilter();

      /// Empty destructor
      ~SimpleAxialStereoSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair& axialStereoSegmentPair);

      /// Returns the trajectory of the axial segment. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const;

      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }

    private:
      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;

    }; // end class SimpleAxialStereoSegmentPairFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SIMPLEAXIALSTEREOSEGMENTPAIRFILTER_H
