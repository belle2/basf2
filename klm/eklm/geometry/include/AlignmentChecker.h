/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMSegmentAlignment.h>
#include <klm/eklm/geometry/Arc2D.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/LineSegment2D.h>
#include <klm/eklm/geometry/Polygon2D.h>

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
      explicit AlignmentChecker(bool printOverlaps);

      /**
       * Copy constructor (disabled).
       */
      AlignmentChecker(const AlignmentChecker&) = delete;

      /**
       * Operator = (disabled).
       */
      AlignmentChecker& operator=(const AlignmentChecker&) = delete;

      /**
       * Destructor.
       */
      ~AlignmentChecker();

      /**
       * Check sector alignment.
       * @param[in] section Section.
       * @param[in] layer   Layer.
       * @param[in] sector  Sector.
       * @param[in] sectorAlignment Sector alignment.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkSectorAlignment(int section, int layer, int sector,
                                const KLMAlignmentData* sectorAlignment) const;

      /**
       * Check segment alignment.
       * @param[in] section               Section.
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
      bool checkSegmentAlignment(int section, int layer, int sector, int plane,
                                 int segment,
                                 const KLMAlignmentData* sectorAlignment,
                                 const KLMAlignmentData* segmentAlignment,
                                 bool calledFromSectorCheck) const;

      /**
       * Check alignment.
       * @param[in] alignment        Alignment data.
       * @param[in] segmentAlignment Segment alignment data.
       * @return true Alignment is correct (no overlaps).
       */
      bool checkAlignment(const EKLMAlignment* alignment,
                          const EKLMSegmentAlignment* segmentAlignment) const;

    private:

      /** Print information about overlaps or not. */
      bool m_PrintOverlaps;

      /** Geometry data. */
      const GeometryData* m_GeoDat;

      /** Element numbers. */
      const EKLMElementNumbers* m_ElementNumbers;

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
