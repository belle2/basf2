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

/* External headers. */
#include <CLHEP/Geometry/Transform3D.h>

/* Belle2 headers. */
#include <eklm/geometry/GeoEKLMTypes.h>

/**
 * @file
 * EKLM geometry data.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct ElementPosition
     * Position information for the elements of detector.
     *
     * @var ElementPosition::InnerR
     * Inner radius.
     *
     * @var ElementPosition::OuterR
     * Outer radius.
     *
     * @var ElementPosition::Length
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
      double InnerR;
      double OuterR;
      double Length;
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
     * @var SectorSupportGeometry::Corner1BInner
     * Corner 1B coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner2Inner
     * Corner 2 coordinates (inner side) (corner of prism, too).
     *
     * @var SectorSupportGeometry::Corner3
     * Corner 3 coordinates.
     *
     * @var SectorSupportGeometry::Corner3Inner
     * Corner 3 coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner3Prism
     * Coordinates of the corner of corner 3 prism.
     *
     * @var SectorSupportGeometry::Corner4
     * Corner 4 coordinates.
     *
     * @var SectorSupportGeometry::Corner4Inner
     * Corner 4 coordinates (inner side).
     *
     * @var SectorSupportGeometry::Corner4Prism
     * Coordinates of the corner of corner 4 prism.
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
      HepGeom::Point3D<double> Corner1A;
      HepGeom::Point3D<double> Corner1AInner;
      HepGeom::Point3D<double> Corner1B;
      HepGeom::Point3D<double> Corner1BInner;
      HepGeom::Point3D<double> Corner2Inner;
      HepGeom::Point3D<double> Corner3;
      HepGeom::Point3D<double> Corner3Inner;
      HepGeom::Point3D<double> Corner3Prism;
      HepGeom::Point3D<double> Corner4;
      HepGeom::Point3D<double> Corner4Inner;
      HepGeom::Point3D<double> Corner4Prism;
    };

    /**
     * Readout board geometry data.
     */
    struct BoardGeometry {
      double Length;      /**< Length. */
      double Width;       /**< Width. */
      double Height;      /**< Height. */
      double BaseWidth;   /**< Width of base board. */
      double BaseHeight;  /**< Height of base board. */
      double StripLength; /**< Length of strip readout board. */
      double StripWidth;  /**< Width of strip readout board. */
      double StripHeight; /**< Height of strip readout board. */
    };

    /**
     * Strip readout board position data.
     */
    struct StripBoardPosition {
      double X;         /**< X coordinate. */
    };

    /**
     * Readout board position data.
     */
    struct BoardPosition {
      double R;      /**< Radius of far edge of the board. */
      double Phi;    /**< Angle. */
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
      double DeltaLRight; /**< Right (X-plane) delta L. */
      double DeltaLLeft;  /**< Left (X-plane) delta L. */
      double Length;      /**< Length */
      double X;           /**< X coordinate. */
      double Y;           /**< Y coordinate. */
      double Z;           /**< Z coordinate. */
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
     * Strip geometry data.
     */
    struct StripGeometry {
      double Width;                    /**< Width. */
      double Thickness;                /**< Thickness. */
      double GrooveDepth;              /**< Groove depth. */
      double GrooveWidth;              /**< Groove width. */
      double NoScintillationThickness; /**< Non-scintillating layer. */
      double RSSSize;                  /**< Radiation study SiPM size. */
    };

    /**
     * Shield layer detail geometry data.
     */
    struct ShieldDetailGeometry {
      double LengthX;                   /**< X length. */
      double LengthY;                   /**< Y length. */
      HepGeom::Point3D<double>* Points; /**< Points. */
    };

    /**
     * Shield layer geometry data.
     */
    struct ShieldGeometry {
      double Thickness;                       /**< Thickness. */
      HepGeom::Point3D<double> DetailACenter; /**< Detail A center. */
      HepGeom::Point3D<double> DetailBCenter; /**< Detail B center. */
      HepGeom::Point3D<double> DetailCCenter; /**< Detail C center. */
      struct ShieldDetailGeometry DetailA;    /**< Detail A. */
      struct ShieldDetailGeometry DetailB;    /**< Detail B. */
      struct ShieldDetailGeometry DetailC;    /**< Detail C. */
      struct ShieldDetailGeometry DetailD;    /**< Detail D. */
    };

    /**
     * EKLM geometry data.
     * All data and function results are in CLHEP units unless noted otherwise.
     */
    class GeometryData {

    public:

      /**
       * Instantiation.
       */
      static const GeometryData& Instance();

      /**
       * Get EKLM detector mode.
       */
      DetectorMode getDetectorMode() const;

      /**
       * Check if displacements are used.
       */
      bool displacedGeometry() const;

      /**
       * Get name of displacement data file.
       */
      std::string getDisplacementDataFile() const;

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
       * Get layer number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @return Number of layer.
       * @details
       * Number is from 1 to 14*2 = 28.
       */
      int layerNumber(int endcap, int layer) const;

      /**
       * Get detector layer number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @return Number of layer.
       * @details
       * Number is from 1 to 12 + 14 = 26.
       */
      int detectorLayerNumber(int endcap, int layer) const;

      /**
       * Get sector number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @return Number of sector.
       * @details
       * Number is from 1 to 4*14*2 = 112.
       */
      int sectorNumber(int endcap, int layer, int sector) const;

      /**
       * Get plane number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       * @return Number of plane.
       * @details
       * Number is from 1 to 2*4*26 = 208.
       */
      int planeNumber(int endcap, int layer, int sector, int plane) const;

      /**
       * Get segment number.
       * @param[in] endcap  Endcap number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       * @param[in] segment Segment number.
       * @return Number of strip.
       * @details
       * Number is from 1 to 5*2*4*26 = 1040.
       */
      int segmentNumber(int endcap, int layer, int sector, int plane,
                        int strip) const;

      /**
       * Get strip number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       * @param[in] strip  Strip number.
       * @return Number of strip.
       * @details
       * Number is from 1 to 75*2*4*26 = 15600.
       */
      int stripNumber(int endcap, int layer, int sector, int plane,
                      int strip) const;

      /**
       * Get maximal strip number.
       */
      int getMaximalStripNumber() const;

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
       * Get strip length.
       * @param[in] strip Strip number.
       */
      double getStripLength(int strip) const;

      /**
       * Get strip geometry data.
       */
      const struct StripGeometry* getStripGeometry() const;

      /**
       * Get shield layer details geometry data.
       */
      const struct ShieldGeometry* getShieldGeometry() const;

      /**
       * Get number of strips with different lengths.
       */
      int getNStripsDifferentLength() const;

      /**
       * Get index in length-based array.
       * @param[in] positionIndex index in position-based array.
       */
      int getStripLengthIndex(int positionIndex) const;

      /**
       * Get index in position-based array.
       * @param[in] positionIndex index in position-based array.
       */
      int getStripPositionIndex(int lengthIndex) const;

      /**
       * Check if z coordinate may be in EKLM.
       * @param[in] z Z coordinate (cm).
       *
       * Warning: this is not a complete check!
       */
      bool hitInEKLM(double z) const;

      /**
       * Get endcap transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of endcap.
       * @details
       * Numbers start from 0.
       */
      void getEndcapTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get layer transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of layer.
       * @details
       * Numbers start from 0.
       */
      void getLayerTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get sector transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of sector.
       * @details
       * Numbers start from 0.
       */
      void getSectorTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get plane transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of plane.
       * @details
       * Numbers start from 0.
       */
      void getPlaneTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get strip transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of strip.
       * @details
       * Numbers start from 0.
       */
      void getStripTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get plastic sheet element transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of list.
       * @details
       * Numbers start from 0.
       * This function is intended for construction of the list solids, so
       * the transformation does not include z shift.
       */
      void getSheetTransform(HepGeom::Transform3D* t, int n) const;

    private:

      /**
       * Constructor.
       */
      GeometryData();

      /**
       * Destructor.
       */
      ~GeometryData();

      /**
       * Calculate sector support geometry data.
       */
      void calculateSectorSupportGeometry();

      /**
       * Fill strip index arrays.
       */
      void fillStripIndexArrays();

      /**
       * Read strip parameters from XML database.
       */
      void readXMLDataStrips();

      /**
       * Calculate shield geometry data.
       */
      void calculateShieldGeometry();

      /** Displacement data file. */
      std::string m_DisplacementDataFile;

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

      /** Number of strips with different lengths in one plane. */
      int m_nStripDifferent;

      /** Number of strip in length-based array. */
      int* m_StripAllToLen;

      /** Number of strip in position-based array. */
      int* m_StripLenToAll;

      /** Number of segments is one plane. */
      int m_nSegment;

      /** Solenoid center Z coordinate. */
      double m_solenoidZ;

      /** Minimal z coordinate of the forward endcap. */
      double m_MinZForward;

      /** Maximal z coordinate of the backward endcap. */
      double m_MaxZBackward;

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

      /** Shield layer details geometry data. */
      struct ShieldGeometry m_ShieldGeometry;

      /** Detector mode. */
      enum DetectorMode m_mode;

    };

  }

}

#endif

