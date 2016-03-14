/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_ALIGNMENTCHECKER_H
#define EKLM_ALIGNMENTCHECKER_H

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/Arc2D.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/LineSegment2D.h>

/**
 * @file
 * EKLM alignment checking.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Class for EKLM alignment checking.
     */
    class AlignmentChecker {

    public:

      /**
       * Constructor.
       */
      AlignmentChecker();

      /**
       * Destructor.
       */
      ~AlignmentChecker();

      /**
       * Check alignment.
       * @param[in] alignment Alignment data.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkAlignment(EKLMAlignment* alignment) const;

    private:

      /**
       * Check segment alignment.
       * @param[in] iPlane    Plane number.
       * @param[in] iSegment  Segment number.
       * @param[in] alignment Alignment data.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkSegmentAlignment(int iPlane, int iSegment,
                                 EKLMAlignmentData* alignment) const;

      /** Geometry data. */
      const GeometryData* m_GeoDat;

      /** Sector support edge: corner 1 line. */
      LineSegment2D* m_LineCorner1;

      /** Sector support edge: outer arc. */
      Arc2D* m_ArcOuter;

      /** Sector support edge: line between corners 2 and 3. */
      LineSegment2D* m_Line23;

      /** Sector support edge: inner arc. */
      Arc2D* m_ArcInner;

      /** Sector support edge: line between corners 4 and 1. */
      LineSegment2D* m_Line41;

    };

  }

}

#endif

