/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLESEGMENTTRIPLEFILTER_H_
#define SIMPLESEGMENTTRIPLEFILTER_H_

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCSZFitter.h>

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of segment triples based on simple criterions
    class SimpleSegmentTripleFilter {

    public:

      /** Constructor. */
      SimpleSegmentTripleFilter();

      /** Destructor.*/
      ~SimpleSegmentTripleFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Checks if a pair of axial segments is a good combination
      bool isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& startToEndAxialAxialSegmentPair);

      /// Checks if a triple of axial, stereo and axial segments is a good combination to be stored as an automaton cell
      /** Checks the proper alignement and the quality of connection between all three segments.
       *  Returns NOT_A_CELL if the connection shall not be made or a finit value be used as the cell weight of the cell to constructed. */
      CellWeight isGoodSegmentTriple(const CDCSegmentTriple& triple);


    private:
      /// Returns the trajectory of the axial segment. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const;

      /// Returns the trajectory of the axial to axial segment piar. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialAxialSegmentPair& axialAxialSegmentPair) const;

      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }


    private:
      /// Returns the sz trajectory of the reconstructed stereo segment of the segment triple. Does a fit if necessary.
      const CDCTrajectorySZ& getFittedTrajectorySZ(const CDCSegmentTriple& segmentTriple) const;

      /// Returns the sz fitter instance that is used by this filter
      const CDCSZFitter& getSZFitter() const
      { return m_szFitter; }

    private:
      CDCRiemannFitter m_riemannFitter; ///< Memory of the Riemann fitter for the circle fits.
      CDCSZFitter m_szFitter; ///< Memory of the SZ fitter fitting sz lines to the stereo segments

    }; // end class SimpleSegmentTripleFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SIMPLESEGMENTTRIPLEFILTER_H_
