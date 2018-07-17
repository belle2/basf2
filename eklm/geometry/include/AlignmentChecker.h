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
#include <eklm/geometry/Polygon2D.h>

/**
 * @file
 * EKLM alignment checking.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Class for EKLM alignment checking.
     * Note that this class internally works in CLHEP units.
     */
    class AlignmentChecker {

    public:

      /**
       * Constructor.
       * @param[in] printOverlaps Print information about overlaps or not.
       */
      AlignmentChecker(bool printOverlaps);

      /**
       * Destructor.
       */
      ~AlignmentChecker();

      /**
       * Check sector alignment.
       * @param[in] endcap Endcap.
       * @param[in] layer  Layer.
       * @param[in] sector Sector.
       * @param[in] sectorAlignment Sector alignment.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkSectorAlignment(int endcap, int layer, int sector,
                                const EKLMAlignmentData* sectorAlignment) const;

      /**
       * Check segment alignment.
       * @param[in] endcap                Endcap.
       * @param[in] layer                 Layer.
       * @param[in] sector                Sector.
       * @param[in] plane                 Plane number.
       * @param[in] segment               Segment number.
       * @param[in] sectorAlignment       Sector alignment.
       * @param[in] segmentAlignment      Segment alignment.
       * @param[in] calledFromSectorCheck Called from checkSectorAlignment()
       *                                  or not.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkSegmentAlignment(int endcap, int layer, int sector, int plane,
                                 int segment,
                                 const EKLMAlignmentData* sectorAlignment,
                                 const EKLMAlignmentData* segmentAlignment,
                                 bool calledFromSectorCheck) const;

      /**
       * Check alignment.
       * @param[in] alignment Alignment data.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkAlignment(const EKLMAlignment* alignment) const;

    private:

      /** Print information about overlaps or not. */
      bool m_PrintOverlaps;

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

      /** Segment support. */
      Polygon2D*** m_SegmentSupport;

    };

  }

}

#endif

