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

#include "BaseAxialAxialSegmentPairFilter.h"
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialAxialSegmentPair.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleAxialAxialSegmentPairFilter : public BaseAxialAxialSegmentPairFilter {

    public:

      /// Constructor
      SimpleAxialAxialSegmentPairFilter();

      /// Empty destructor
      virtual ~SimpleAxialAxialSegmentPairFilter() {;}

    public:

      /// Clears all remember information from the last event
      virtual void clear() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() override final {;}

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair) override final;

    public:
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


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //SIMPLEAXIALAXIALSEGMENTPAIRFILTER_H_
