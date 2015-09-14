/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_GEOMETRYDATA2_H
#define EKLM_GEOMETRYDATA2_H

/**
 * @file
 * EKLM geometry data.
 */

/* Belle2 headers. */
#include <eklm/geometry/GeoEKLMTypes.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct Point2D
     * 2D point.
     *
     * @var Point2D::X
     * X coordinate.
     *
     * @var Point2D::Y
     * Y coordinate.
     */
    struct Point2D {
      double X;
      double Y;
    };

    /**
     * @struct ElementPosition
     * Position information for the elements of detector.
     *
     * @var ElementPosition::innerR
     * Inner radius.
     *
     * @var ElementPosition::outerR
     * Outer radius.
     *
     * @var ElementPosition::length
     * Length.
     *
     * @var ElementPosition::X
     * X coordinate.
     *
     * @var ElementPosition::Y
     * Y coordinate.
     *
     * @var ElementPosition::Z
     * Z coordinate.
     *
     */
    struct ElementPosition {
      double innerR;
      double outerR;
      double length;
      double X;
      double Y;
      double Z;
    };

    /**
     * @struct SectorSupportGeometry
     * Sector support geometry data.
     *
     * @var SectorSupportGeometry::Thickness
     * Thickness.
     *
     * @var SectorSupportGeometry::DeltaLY
     * outerR - Y of upper edge of BoxY.
     *
     * @var SectorSupportGeometry::CornerX
     * Coordinate X of corner 1.
     *
     * @var SectorSupportGeometry::Corner1LX
     * Corner 1 X length.
     *
     * @var SectorSupportGeometry::Corner1Width
     * Corner 1 width.
     *
     * @var SectorSupportGeometry::Corner1Thickness
     * Corner 1 thickness.
     *
     * @var SectorSupportGeometry::Corner1Z
     * Corner 1 Z coordinate.
     *
     * @var SectorSupportGeometry::Corner2LX
     * Corner 2 X length.
     *
     * @var SectorSupportGeometry::Corner2LY
     * Corner 2 Y length.
     *
     * @var SectorSupportGeometry::Corner2Thickness
     * Corner 2 thickness.
     *
     * @var SectorSupportGeometry::Corner2Z
     * Corner 2 Z coordinate.
     *
     * @var SectorSupportGeometry::Corner3LX
     * Corner 3 X length.
     *
     * @var SectorSupportGeometry::Corner3LY
     * Corner 3 Y length.
     *
     * @var SectorSupportGeometry::Corner3Thickness
     * Corner 3 thickness.
     *
     * @var SectorSupportGeometry::Corner3Z
     * Corner 3 Z coordinate.
     *
     * @var SectorSupportGeometry::Corner4LX
     * Corner 4 X length.
     *
     * @var SectorSupportGeometry::Corner4LY
     * Corner 4 Y length.
     *
     * @var SectorSupportGeometry::Corner4Thickness
     * Corner 4 thickness.
     *
     * @var SectorSupportGeometry::Corner4Z
     * Corner 4 Z coordinate.
     *
     * @var SectorSupportGeometry::CornerAngle
     * Corner 1 angle.
     *
     * @var SectorSupportGeometry::Corner1A
     * Corner 1A coordinates.
     *
     * @var SectorSupportGeometry::Corner1AInner
     * Corner 1A coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner1B
     * Corner 1B coordinates.
     *
     * @var SectorSupportGeometry::Corner3
     * Corner 3 coordinates.
     *
     * @var SectorSupportGeometry::Corner4
     * Corner 4 coordinates.
     */
    struct SectorSupportGeometry {
      double Thickness;
      double DeltaLY;
      double CornerX;
      double Corner1LX;
      double Corner1Width;
      double Corner1Thickness;
      double Corner1Z;
      double Corner2LX;
      double Corner2LY;
      double Corner2Thickness;
      double Corner2Z;
      double Corner3LX;
      double Corner3LY;
      double Corner3Thickness;
      double Corner3Z;
      double Corner4LX;
      double Corner4LY;
      double Corner4Thickness;
      double Corner4Z;
      double CornerAngle;
      Point2D Corner1A;
      Point2D Corner1AInner;
      Point2D Corner1B;
      Point2D Corner3;
      Point2D Corner4;
    };

    /**
     * Readout board geometry data.
     */
    struct BoardGeometry {
      double length;       /**< Length. */
      double width;        /**< Width. */
      double height;       /**< Height. */
      double base_width;   /**< Width of base board. */
      double base_height;  /**< Height of base board. */
      double strip_length; /**< Length of strip readout board. */
      double strip_width;  /**< Width of strip readout board. */
      double strip_height; /**< Height of strip readout board. */
    };

    /**
     * Strip readout board position data.
     */
    struct StripBoardPosition {
      double x;         /**< X coordinate. */
    };

    /**
     * Readout board position data.
     */
    struct BoardPosition {
      double r;      /**< Radius of far edge of the board. */
      double phi;    /**< Angle. */
    };

    /**
     * @struct SegmentSupportGeometry
     * Segment support geometry data.
     *
     * @var SegmentSupportGeometry::TopWidth
     * Top box width.
     *
     * @var SegmentSupportGeometry::TopThickness
     * Top box thickness.
     *
     * @var SegmentSupportGeometry::MiddleWidth
     * Middle box width.
     *
     * @var SegmentSupportGeometry::MiddleThickness
     * Middle box thickness.
     */
    struct SegmentSupportGeometry {
      double TopWidth;
      double TopThickness;
      double MiddleWidth;
      double MiddleThickness;
    };

    /**
     * Segment support position.
     */
    struct SegmentSupportPosition {
      double deltal_right;  /**< Right (X-plane) delta L. */
      double deltal_left;   /**< Left (X-plane) delta L. */
      double length;        /**< Length */
      double x;             /**< X coordinate. */
      double y;             /**< Y coordinate. */
      double z;             /**< Z coordinate. */
    };

    /**
     * @struct PlasticSheetGeometry
     * Plastic sheet geometry data.
     *
     * @var PlasticSheetGeometry::Width
     * Width.
     *
     * @var PlasticSheetGeometry::DeltaL
     * Distance from edge of last strip to edge of plastic list.
     */
    struct PlasticSheetGeometry {
      double Width;
      double DeltaL;
    };

    /**
     * Strip size data.
     */
    struct StripGeometry {
      double width;                      /**< Width. */
      double thickness;                  /**< Thickness. */
      double groove_depth;               /**< Groove depth. */
      double groove_width;               /**< Groove width. */
      double no_scintillation_thickness; /**< Non-scintillating layer. */
      double rss_size;                   /**< Radiation study SiPM size. */
    };

    /**
     * EKLM geometry data.
     */
    class GeometryData2 {

    public:

      /**
       * Instantiation.
       */
      static const GeometryData2& Instance();

      /**
       * Get EKLM detector mode.
       */
      EKLMDetectorMode getDetectorMode() const;

      /**
       * Get number of layers.
       */
      int getNLayers() const;

      /**
       * Get number of detector layers.
       * @param[in] endcap Endcap number (1 - backward, 2 - forward).
       */
      int getNDetectorLayers(int endcap) const;

      /**
       * Get number of planes.
       */
      int getNPlanes() const;

      /**
       * Get number of readout boards.
       */
      int getNBoards() const;

      /**
       * Get number of strip readout boards.
       */
      int getNStripBoards() const;

      /**
       * Get number of strips.
       */
      int getNStrips() const;

      /**
       * Get number of segments.
       */
      int getNSegments() const;

      /**
       * Get solenoid center Z coordinate.
       */
      double getSolenoidZ() const;

      /**
       * Get position data for endcaps.
       */
      const struct ElementPosition* getEndcapPosition() const;

      /**
       * Get position data for layers.
       */
      const struct ElementPosition* getLayerPosition() const;

      /**
       * Get Z distance between two layers.
       */
      double getLayerShiftZ() const;

      /**
       * Get position data for sectors.
       */
      const struct ElementPosition* getSectorPosition() const;

      /**
       * Get position data for sector support structure.
       */
      const struct ElementPosition* getSectorSupportPosition() const;

      /**
       * Get sector support geometry data.
       */
      const struct SectorSupportGeometry* getSectorSupportGeometry() const;

      /**
       * Get readout board geometry data.
       */
      const struct BoardGeometry* getBoardGeometry() const;

      /**
       * Get position data for readout boards.
       * @param[in] plane   Plane number.
       * @param[in] segment Segment number.
       */
      const struct BoardPosition*
      getBoardPosition(int plane, int segment) const;

      /**
       * Get position data for strip readout boards.
       * @param[in] board Number of board.
       */
      const struct StripBoardPosition* getStripBoardPosition(int board) const;

      /**
       * Get position data for planes.
       */
      const struct ElementPosition* getPlanePosition() const;

      /**
       * Get position data for segment support structure.
       * @param[in] plane   Plane number.
       * @param[in] support Segment support element number.
       */
      const struct SegmentSupportPosition*
      getSegmentSupportPosition(int plane, int support) const;

      /**
       * Get segment support geometry data.
       */
      const struct SegmentSupportGeometry* getSegmentSupportGeometry() const;

      /**
       * Get plastic sheet geometry data.
       */
      const struct PlasticSheetGeometry* getPlasticSheetGeometry() const;

      /**
       * Get position data for strips.
       * @param[in] strip Strip number.
       */
      const struct ElementPosition* getStripPosition(int strip) const;

      /**
       * Get strip geometry data.
       */
      const struct StripGeometry* getStripGeometry() const;

    private:

      /**
       * Constructor.
       */
      GeometryData2();

      /**
       * Destructor.
       */
      ~GeometryData2();

      /**
       * Calculate sector support geometry data.
       */
      void calculateSectorSupportGeometry();

      /**
       * Read strip parameters from XML database.
       */
      void readXMLDataStrips();

      /** Number of layers. */
      int m_nLayer;

      /** Number of detector layers in the forward endcap. */
      int m_nLayerForward;

      /** Number of detector layers in the backward endcap. */
      int m_nLayerBackward;

      /** Number of planes in one sector. */
      int m_nPlane;

      /** Number of readout boards in one sector. */
      int m_nBoard;

      /** Number of strip readout boards on one segment readout board. */
      int m_nStripBoard;

      /** Number of strips in one plane. */
      int m_nStrip;

      /** Number of segments is one plane. */
      int m_nSegment;

      /** Solenoid center Z coordinate. */
      double m_solenoidZ;

      /** Position data for endcaps. */
      struct ElementPosition m_EndcapPosition;

      /** Position data for layers. */
      struct ElementPosition m_LayerPosition;

      /** Z distance between two layers. */
      double m_LayerShiftZ;

      /** Position data for sectors. */
      struct ElementPosition m_SectorPosition;

      /** Position data for sector support structure. */
      struct ElementPosition m_SectorSupportPosition;

      /** Sector support geometry data. */
      struct SectorSupportGeometry m_SectorSupportGeometry;

      /** Readout board geometry data. */
      struct BoardGeometry m_BoardGeometry;

      /** Positions of readout boards. */
      struct BoardPosition* m_BoardPosition[2];

      /** Positions of strip readout boards. */
      struct StripBoardPosition* m_StripBoardPosition;

      /** Position data for planes. */
      struct ElementPosition m_PlanePosition;

      /** Position data for segment support structure. */
      struct SegmentSupportPosition* m_SegmentSupportPosition[2];

      /** Segment support geometry data. */
      struct SegmentSupportGeometry m_SegmentSupportGeometry;

      /** Plastic sheet geometry data. */
      struct PlasticSheetGeometry m_PlasticSheetGeometry;

      /** Position data for strips. */
      struct ElementPosition* m_StripPosition;

      /** Strip size data. */
      struct StripGeometry m_StripGeometry;

      /** Detector mode. */
      enum EKLMDetectorMode m_mode;

    };

  }

}

#endif

