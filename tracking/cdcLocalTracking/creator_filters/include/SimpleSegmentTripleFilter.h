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
      CellWeight isGoodTriple(const CDCAxialRecoSegment2D& startSegment,
                              const CDCStereoRecoSegment2D& middleSegment,
                              const CDCAxialRecoSegment2D& endSegment);

      /// Set the circle fit and the sz of the segment triple to be stored as a cell
      void setTrajectoryOf(const CDCSegmentTriple& segmentTriple);

    private:
      /// Type to store fits to segments
      typedef std::map<const CDCAxialRecoSegment2D*, CDCTrajectory2D > SegmentXYFitMap;

      /// A pair of the segments to store. Used in the in the map to store fits to pairs of axial segments
      typedef std::pair<const CDCAxialRecoSegment2D*, const CDCAxialRecoSegment2D*> SegmentPair;

      /// Type to store fits to pairs of axial segments
      typedef std::map<SegmentPair, CDCTrajectory2D > SegmentPairXYFitMap;

      /// An axial, stereo, axial segment triple to be used in the map storing sz fits
      typedef boost::tuple<const CDCAxialRecoSegment2D*, const CDCStereoRecoSegment2D*, const CDCAxialRecoSegment2D*>  ASATriple;

      /// Map type to store sz fits to stereo segment supported by two surrounding axial segments
      typedef std::map<ASATriple, CDCTrajectorySZ > SegmentTripleSZFitMap;


      SegmentXYFitMap m_segmentXYFits; ///< Memory of the map storing xy fits to the single axial segments
      SegmentPairXYFitMap m_segmentPairXYFits; ///< Memory of the map storing xy fits to pairs of axial segments
      SegmentTripleSZFitMap m_segmentTripleSZFits; ///< Memory of the map storing sz fits to stereo segments supported by two axial segments

      CDCRiemannFitter m_xyFitter; ///< Memory of the Riemann fitter for the circle fits.
      CDCSZFitter m_szFitter; ///< Memory of the SZ fitter fitting sz lines to the stereo segments


      /// Getter for an circle fit to an axial segments
      /** Returns the circle fit to an axial segment. The fit gets stored in the fit map and is returned without
       *  recalculation if the fit to the same segment is requested again */
      const CDCTrajectory2D& getXYFit(const CDCAxialRecoSegment2D& segment);


      /** Returns the circle fit to a pair of axial segments. The fit gets stored in the fit map and is returned without
       *  recalculation if the fit to the same two segments is requested again */
      const CDCTrajectory2D& getXYFit(const CDCAxialRecoSegment2D& startSegment,
                                      const CDCAxialRecoSegment2D& endSegment);

      /** Returns the sz fit to a triple of segments. The fit gets stored in the fit map and is returned without
       *  recalculation, if the fit to the same three segments is requested again */
      const CDCTrajectorySZ& getSZFit(
        const CDCAxialRecoSegment2D& startSegment,
        const CDCStereoRecoSegment2D& middleSegment,
        const CDCAxialRecoSegment2D& endSegment
      );

      /** Manually store a sz fit of the three segments. The fit can be obtained by the getSZFit function again. */
      void storeSZFit(const CDCAxialRecoSegment2D& startSegment,
                      const CDCStereoRecoSegment2D& middleSegment,
                      const CDCAxialRecoSegment2D& endSegment,
                      const CDCTrajectorySZ& szFit);


    }; // end class SimpleSegmentTripleFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SIMPLESEGMENTTRIPLEFILTER_H_
