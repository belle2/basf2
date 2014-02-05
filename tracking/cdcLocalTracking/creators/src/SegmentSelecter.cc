/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/SegmentSelecter.h"

//#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/logging/Logger.h>

#include <deque>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


SegmentSelecter::SegmentSelecter()
{
}

SegmentSelecter::~SegmentSelecter()
{
}


void SegmentSelecter::selectSegments(vector<CDCRecoSegment2D>& in,
                                     vector<CDCRecoSegment2D>& out) const
{




  typedef pair<CDCTrajectory2D, double> PairFitChi2;
  typedef map<CDCRecoSegment2D*, PairFitChi2 > MapRecoSegmentsToFits;
  MapRecoSegmentsToFits mapRecoSegmentsToFits;

  CDCRiemannFitter positionFitter;
  positionFitter.useOnlyPosition();

  CDCRiemannFitter orientationFitter;
  orientationFitter.useOnlyOrientation();

  CDCRiemannFitter bothFitter;
  orientationFitter.usePositionAndOrientation();

  for (vector<CDCRecoSegment2D>::iterator itRecoSegment = in.begin();
       itRecoSegment != in.end(); ++itRecoSegment) {
    CDCRecoSegment2D& recosegment = *itRecoSegment;

    PairFitChi2& fitandchi2 = mapRecoSegmentsToFits[&recosegment];
    CDCTrajectory2D& circlefit = fitandchi2.first;

    //now make the fit
    //CDCGeneralizedCircleFit positionFit;
    //positionFitter.update(positionFit,recosegment);
    //B2DEBUG(200," Squared distance / #hits of position fit " << recosegment.squaredDistanceTo(positionFit));

    //CDCGeneralizedCircleFit orientationFit;
    //orientationFitter.update(orientationFit,recosegment);
    //B2DEBUG(200," Squared distance / #hits of orientatin fit " << recosegment.squaredDistanceTo(orientationFit));

    CDCTrajectory2D& bothFit = circlefit;
    bothFitter.update(bothFit, recosegment);
    //B2DEBUG(200," Squared distance / #hits of both fit " << recosegment.squaredDistanceTo(bothFit));

    //assoziate the chi2 with the segment
    fitandchi2.second = recosegment.getSquaredDist2D(circlefit);
    //B2DEBUG(200," Squared distance / #hits of final fit " << fitandchi2.second);

  }

  //register the segments to compete about the wirehits
  //define an appropriate type to map the hits to the segments
  typedef map< const CDCWireHit*, vector< CDCRecoSegment2D* > > WireHitToSegmentMap;

  WireHitToSegmentMap hitAssignedTo;

  for (vector<CDCRecoSegment2D>::iterator itRecoSegment = in.begin();
       itRecoSegment != in.end(); ++itRecoSegment) {

    CDCRecoSegment2D& segment = *itRecoSegment;

    //register the hits of the segment to be competed about
    for (CDCRecoSegment2D::iterator itRecoHit = segment.begin();
         itRecoHit != segment.end(); ++itRecoHit) {

      const CDCWireHit* wirehit = itRecoHit->getWireHit();
      hitAssignedTo[wirehit].push_back(&segment);

    }
  }

  for (WireHitToSegmentMap::iterator  itWireHitSegmentsPair = hitAssignedTo.begin();
       itWireHitSegmentsPair != hitAssignedTo.end();  ++itWireHitSegmentsPair) {


    //const CDCWireHit* wirehit = itWireHitSegmentsPair->first;
    vector< CDCRecoSegment2D* >& segments = itWireHitSegmentsPair->second;


    vector< CDCRecoSegment2D* >::iterator itWinningSegment = segments.begin();

    for (vector< CDCRecoSegment2D* >::iterator itSegment = segments.begin();
         itSegment != segments.end() ; ++itSegment) {

      CDCRecoSegment2D* winningSegment = *itWinningSegment;
      //PairFitChi2& winningFitAndChi2 = mapRecoSegmentsToFits[winningSegment];
      //const double & winningChi2 = winningFitAndChi2.second;
      size_t winningSize = winningSegment->size();

      CDCRecoSegment2D* segment = *itSegment;
      //PairFitChi2& fitAndChi2 = mapRecoSegmentsToFits[segment];
      //const double& chi2 = fitAndChi2.second;
      size_t size = segment->size();

      //scoring rule
      if (winningSize <  size) {
        itWinningSegment = itSegment;
      }

    }

    CDCRecoSegment2D* winningSegment = itWinningSegment == segments.end() ? nullptr : *itWinningSegment;
    segments.clear();
    segments.push_back(winningSegment);
  }

  //iter over the segments and building new segments with the winning hits
  for (vector<CDCRecoSegment2D>::iterator itSegment = in.begin();
       itSegment != in.end(); ++itSegment) {

    CDCRecoSegment2D& segment = *itSegment;

    out.push_back(CDCRecoSegment2D());
    CDCRecoSegment2D& newSegment = out.back();

    for (CDCRecoSegment2D::iterator itRecoHit = segment.begin();
         itRecoHit != segment.end(); ++itRecoHit) {

      CDCRecoHit2D& recohit = *itRecoHit;
      const CDCWireHit* wirehit = recohit.getWireHit();
      if (hitAssignedTo[wirehit][0] == &segment) {
        newSegment.push_back(recohit);
      }
    }
    if (newSegment.size() < 3) out.pop_back();
  }
}
