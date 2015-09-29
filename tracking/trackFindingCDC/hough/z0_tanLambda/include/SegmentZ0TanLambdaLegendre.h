/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitInZ0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/RangeInBox.h>
#include <tracking/trackFindingCDC/hough/SimpleBoxDivisionHoughTree.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<size_t z0Divisions = 2,
             size_t tanLambdaDivisions = 2>
    class SegmentZ0TanLambdaLegendre : public
      SimpeBoxDivisionHoughTree<const CDCRecoSegment3D*, RangeInBox<HitInZ0TanLambdaBox>, z0Divisions, tanLambdaDivisions> {

    private:
      /// Super type
      typedef SimpeBoxDivisionHoughTree<const CDCRecoSegment3D*, RangeInBox<HitInZ0TanLambdaBox>, z0Divisions, tanLambdaDivisions> Super;
    public:
      /// Constructor using the given maximal level setting the maximal values.
      SegmentZ0TanLambdaLegendre(size_t maxLevel) : Super(maxLevel, 120, std::tan(75.0 * M_PI / 180.0)) {}
    };
  }
}
