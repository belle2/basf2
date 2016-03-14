/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCAXIALAXIALSEGMENTPAIRFILTER_H_
#define MCAXIALAXIALSEGMENTPAIRFILTER_H_

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/rootification/SwitchableRootificationBase.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialAxialSegmentPair.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialAxialSegmentPairFilter : public SwitchableRootificationBase {

    public:

      /// Constructor
      MCAxialAxialSegmentPairFilter();

      /// Empty destructor
      ~MCAxialAxialSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialAxialSegmentPair(const Belle2::CDCLocalTracking::CDCAxialAxialSegmentPair& axialAxialSegmentPair, bool allowBackward = false) const;

    private:
      /// ROOT Macro to make MCAxialAxialSegmentPairFilter a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(MCAxialAxialSegmentPairFilter, 1);

    }; // end class MCAxialAxialSegmentPairFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // MCAXIALAXIALSEGMENTPAIRFILTER_H_
