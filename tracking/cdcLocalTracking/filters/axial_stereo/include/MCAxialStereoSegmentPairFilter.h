/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCAXIALSTEREOSEGMENTPAIRFILTER_H
#define MCAXIALSTEREOSEGMENTPAIRFILTER_H

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialStereoSegmentPair.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to stereo segment pairs based on Monte Carlo information.
    class MCAxialStereoSegmentPairFilter : public SwitchableRootificationBase {

    public:

      /// Constructor
      MCAxialStereoSegmentPairFilter();

      /// Empty destructor
      ~MCAxialStereoSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Checks if a axial stereo segment pair is a good combination.
      CellWeight isGoodAxialStereoSegmentPair(const Belle2::CDCLocalTracking::CDCAxialStereoSegmentPair& axialAxialSegmentPair,
                                              bool allowBackward = false) const;

    private:
      /// ROOT Macro to make MCAxialStereoSegmentPairFilter a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(MCAxialStereoSegmentPairFilter, 1);

    }; // end class MCAxialStereoSegmentPairFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // MCAXIALSTEREOSEGMENTPAIRFILTER_H
