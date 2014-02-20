/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef AXIALAXIALSEGMENTPAIRFILTERVARS_H_
#define AXIALAXIALSEGMENTPAIRFILTERVARS_H_

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialAxialSegmentPair.h>

#include <root/TVector2.h>
#include <root/TObject.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class AxialAxialSegmentPairFilterVars : public TObject {

    public:
      /// Constructor setting up an out file and a tree to be filled
      AxialAxialSegmentPairFilterVars();

      /// Deconstructor writing the tree to disk and closing the file
      ~AxialAxialSegmentPairFilterVars();

    public:
      /// Fills the object with the information gather from the given segment to segment pair instance.
      bool fill(const CDCAxialAxialSegmentPair& axialAxialSegmentPair);

      Float_t m_startSegment_startPerpS_startFit;
      Float_t m_startSegment_endPerpS_startFit;

      Float_t m_endSegment_startPerpS_startFit;
      Float_t m_endSegment_endPerpS_startFit;

      Float_t m_startSegment_startPerpS_endFit;
      Float_t m_startSegment_endPerpS_endFit;

      Float_t m_endSegment_startPerpS_endFit;
      Float_t m_endSegment_endPerpS_endFit;

      TVector2 m_startSegment_com;
      TVector2 m_endSegment_com;

      Float_t m_startSegment_mom;
      Float_t m_endSegment_mom;

      TVector2 m_startSegment_center;
      TVector2 m_endSegment_center;

      TVector2 m_startSegment_extrapolation;
      TVector2 m_endSegment_extrapolation;

      TVector2 m_startSegment_centerMom;
      TVector2 m_endSegment_centerMom;

      TVector2 m_startSegment_extarpolationMom;
      TVector2 m_endSegment_extrapolationMom;

      Float_t m_distanceAtStart;
      Float_t m_distanceAtEnd;

      Float_t m_momAngleDeviationAtStart;
      Float_t m_momAngleDeviationAtEnd;


    private:
      /// ROOT Macro to make AxialAxialSegmentPairFilterVars a ROOT class.
      ClassDef(AxialAxialSegmentPairFilterVars, 1);

    }; // end class AxialAxialSegmentPairFilterVars

  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // AXIALAXIALSEGMENTPAIRFILTERVARS_H_
