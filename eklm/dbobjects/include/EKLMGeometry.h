/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMGEOMETRY_H
#define EKLMGEOMETRY_H

/* External headers. */
#include <TObject.h>
#include <CLHEP/Geometry/Point3D.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMElementNumbers.h>

namespace Belle2 {

  /**
   * Class to store EKLM geometry data in the database.
   */
  class EKLMGeometry : public EKLMElementNumbers {

  public:

    /** Mode of detector operation. */
    enum DetectorMode {
      /** Normal mode. */
      c_DetectorNormal = 0,
      /** Background study. */
      c_DetectorBackground = 1,
    };

    /**
     * @struct EndcapStructureGeometry
     * @brief EndcapStructure geometry parameters.
     *
     * @var EndcapStructureGeometry::Phi
     * The starting angle of the octagonal Endcap KLM shape.
     *
     * @var EndcapStructureGeometry::Dphi
     * The opening angle (shape is extended from phi to phi+dphi).
     *
     * @var EndcapStructureGeometry::Nsides
     * The number of sides (=8 : octagonal).
     *
     * @var EndcapStructureGeometry::Nboundary
     * The number of boundaries perpendicular to the z-axis.
     *
     * @var EndcapStructureGeometry::Z
     * The z coordinate of the section specified by input id (=boundary id).
     *
     * @var EndcapStructureGeometry::Rmin
     * The radius of the circle tangent to the sides of the inner polygon.
     *
     * @var EndcapStructureGeometry::Rmax
     * The radius of the circle tangent to the sides of the outer polygon.
     *
     * @var EndcapStructureGeometry::Zsub
     * The length of the tube.
     *
     * @var EndcapStructureGeometry::Rminsub
     * The inner radius of the tube.
     *
     * @var EndcapStructureGeometry::Rmaxsub
     * The outer radius of the tube.
     */
    struct EndcapStructureGeometry : public TObject {

      /**
       * Constructor.
       */
      EndcapStructureGeometry();

      /**
       * Copy constructor.
       */
      EndcapStructureGeometry(const EndcapStructureGeometry& geometry);

      /**
       * Operator =.
       */
      EndcapStructureGeometry& operator=(
        const EndcapStructureGeometry& geometry);

      double Phi;
      double Dphi;
      int Nsides;
      int Nboundary;
      double* Z;    //[Nboundary]
      double* Rmin; //[Nboundary]
      double* Rmax; //[Nboundary]
      double Zsub;
      double Rminsub;
      double Rmaxsub;

    private:

      /** Makes objects storable. */
      ClassDef(EndcapStructureGeometry, 1);

    };

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
    struct ElementPosition : public TObject {

      /**
       * Constructor.
       */
      ElementPosition();

      double InnerR;
      double OuterR;
      double Length;
      double X;
      double Y;
      double Z;

    private:

      /** Makes objects storable. */
      ClassDef(ElementPosition, 1);

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
    struct SectorSupportGeometry : public TObject {

      /**
       * Constructor.
       */
      SectorSupportGeometry();

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
      /* The following parameters are not stored in the database. */
      double CornerAngle;                     //!
      HepGeom::Point3D<double> Corner1A;      //!
      HepGeom::Point3D<double> Corner1AInner; //!
      HepGeom::Point3D<double> Corner1B;      //!
      HepGeom::Point3D<double> Corner1BInner; //!
      HepGeom::Point3D<double> Corner2Inner;  //!
      HepGeom::Point3D<double> Corner3;       //!
      HepGeom::Point3D<double> Corner3Inner;  //!
      HepGeom::Point3D<double> Corner3Prism;  //!
      HepGeom::Point3D<double> Corner4;       //!
      HepGeom::Point3D<double> Corner4Inner;  //!
      HepGeom::Point3D<double> Corner4Prism;  //!

    private:

      /** Makes objects storable. */
      ClassDef(SectorSupportGeometry, 1);

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
    struct PlasticSheetGeometry : public TObject {

      /**
       * Constructor.
       */
      PlasticSheetGeometry();

      double Width;
      double DeltaL;

    private:

      /** Makes objects storable. */
      ClassDef(PlasticSheetGeometry, 1);

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
    struct SegmentSupportGeometry : public TObject {

      /**
       * Constructor.
       */
      SegmentSupportGeometry();

      double TopWidth;
      double TopThickness;
      double MiddleWidth;
      double MiddleThickness;

    private:

      /** Makes objects storable. */
      ClassDef(SegmentSupportGeometry, 1);

    };

    /**
     * Segment support position.
     */
    struct SegmentSupportPosition : public TObject {

      /**
       * Constructor.
       */
      SegmentSupportPosition();

      double DeltaLRight; /**< Right (X-plane) delta L. */
      double DeltaLLeft;  /**< Left (X-plane) delta L. */
      double Length;      /**< Length */
      double X;           /**< X coordinate. */
      double Y;           /**< Y coordinate. */
      double Z;           /**< Z coordinate. */

    private:

      /** Makes objects storable. */
      ClassDef(SegmentSupportPosition, 1);

    };

    /**
     * Strip geometry data.
     */
    struct StripGeometry : public TObject {

      /**
       * Constructor.
       */
      StripGeometry();

      double Width;                    /**< Width. */
      double Thickness;                /**< Thickness. */
      double GrooveDepth;              /**< Groove depth. */
      double GrooveWidth;              /**< Groove width. */
      double NoScintillationThickness; /**< Non-scintillating layer. */
      double RSSSize;                  /**< Radiation study SiPM size. */

    private:

      /** Makes objects storable. */
      ClassDef(StripGeometry, 1);

    };

    /**
     * 2D point.
     */
    struct Point : public TObject {

      /**
       * Constructor.
       */
      Point();

      double X; /**< X coordinate. */
      double Y; /**< Y coordinate. */

    private:

      /** Makes objects storable. */
      ClassDef(Point, 1);

    };

    /**
     * Shield layer detail geometry data.
     */
    struct ShieldDetailGeometry : public TObject {

      /**
       * Constructor.
       */
      ShieldDetailGeometry();

      /**
       * Copy constructor.
       */
      ShieldDetailGeometry(const ShieldDetailGeometry& geometry);

      /**
       * Operator =.
       */
      ShieldDetailGeometry& operator=(const ShieldDetailGeometry& geometry);

      /** X length. */
      double LengthX;

      /** Y length. */
      double LengthY;

      /** Number of points. */
      int NPoints;

      /** Points. */
      Point* Points; //[NPoints]

    private:

      /** Makes objects storable. */
      ClassDef(ShieldDetailGeometry, 1);

    };

    /**
     * @struct ShieldGeometry
     * Shield layer geometry data.
     *
     * @var ShieldGeometry::Thickness
     * Thickness.
     *
     * @var ShieldGeometry::DetailA
     * Detail A.
     *
     * @var ShieldGeometry::DetailB
     * Detail B.
     *
     * @var ShieldGeometry::DetailC
     * Detail C.
     *
     * @var ShieldGeometry::DetailD
     * Detail D.
     *
     * @var ShieldGeometry::DetailACenter
     * Detail A center.
     *
     * @var ShieldGeometry::DetailBCenter
     * Detail B center.
     *
     * @var ShieldGeometry::DetailCCenter
     * Detail C center.
     */
    struct ShieldGeometry : public TObject {

      /**
       * Constructor.
       */
      ShieldGeometry();

      double Thickness;
      struct ShieldDetailGeometry DetailA;
      struct ShieldDetailGeometry DetailB;
      struct ShieldDetailGeometry DetailC;
      struct ShieldDetailGeometry DetailD;
      /* The following parameters are not stored in the database. */
      HepGeom::Point3D<double> DetailACenter; //!
      HepGeom::Point3D<double> DetailBCenter; //!
      HepGeom::Point3D<double> DetailCCenter; //!

    private:

      /** Makes objects storable. */
      ClassDef(ShieldGeometry, 1);

    };

    /**
     * Readout board geometry data.
     */
    struct BoardGeometry : public TObject {

      /**
       * Constructor.
       */
      BoardGeometry();

      double Length;      /**< Length. */
      double Width;       /**< Width. */
      double Height;      /**< Height. */
      double BaseWidth;   /**< Width of base board. */
      double BaseHeight;  /**< Height of base board. */
      double StripLength; /**< Length of strip readout board. */
      double StripWidth;  /**< Width of strip readout board. */
      double StripHeight; /**< Height of strip readout board. */

    private:

      /** Makes objects storable. */
      ClassDef(BoardGeometry, 1);

    };

    /**
     * Readout board position data.
     */
    struct BoardPosition : public TObject {

      /**
       * Constructor.
       */
      BoardPosition();

      double R;      /**< Radius of far edge of the board. */
      double Phi;    /**< Angle. */

    private:

      /** Makes objects storable. */
      ClassDef(BoardPosition, 1);

    };

    /**
     * Strip readout board position data.
     */
    struct StripBoardPosition : public TObject {

      /**
       * Constructor.
       */
      StripBoardPosition();

      double X;         /**< X coordinate. */

    private:

      /** Makes objects storable. */
      ClassDef(StripBoardPosition, 1);

    };

    /**
     * Constructor.
     */
    EKLMGeometry();

    /**
     * Copy constructor.
     */
    EKLMGeometry(const EKLMGeometry& geometry);

    /**
     * Destructor.
     */
    ~EKLMGeometry();

    /**
     * Operator =.
     */
    EKLMGeometry& operator=(const EKLMGeometry& geometry);

    /**
     * Get EKLM detector mode.
     */
    enum DetectorMode getDetectorMode() const;

    /**
     * Get number of endcaps.
     */
    int getNEndcaps() const;

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
     * Get number of sectors.
     */
    int getNSectors() const;

    /**
     * Get number of planes.
     */
    int getNPlanes() const;

    /**
     * Get number of segments.
     */
    int getNSegments() const;

    /**
     * Get number of segment support elements *in one sector).
     */
    int getNSegmentSupportElementsSector() const;

    /**
     * Get number of strips in a segment.
     */
    int getNStripsSegment() const;

    /**
     * Get number of strips.
     */
    int getNStrips() const;

    /**
     * Get number of readout boards.
     */
    int getNBoards() const;

    /**
     * Get number of redout boards in one sector.
     */
    int getNBoardsSector() const;

    /**
     * Get number of strip readout boards.
     */
    int getNStripBoards() const;

    /**
     * Check if number of detector layers is correct (fatal error if not).
     * Endcap number must be checked separately.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     */
    void checkDetectorLayerNumber(int endcap, int layer) const;

    /**
     * Check if detector layer number is correct (fatal error if not).
     * Endcap number must be checked separately.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     */
    void checkDetectorLayer(int endcap, int layer) const;

    /**
     * Check if segment support number is correct (fatal error if not).
     * @param[in] support Segment support element number.
     */
    void checkSegmentSupport(int support) const;

    /**
     * Check if number of strip in a segment is correct (fatal error if not).
     * @param[in] strip Strip number.
     */
    void checkStripSegment(int strip) const;

    /**
     * Get solenoid center Z coordinate.
     */
    double getSolenoidZ() const;

    /**
     * Get endcap structure geometry data.
     */
    const struct EndcapStructureGeometry* getEndcapStructureGeometry() const;

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
     * Get position data for planes.
     */
    const struct ElementPosition* getPlanePosition() const;

    /**
     * Get plastic sheet geometry data.
     */
    const struct PlasticSheetGeometry* getPlasticSheetGeometry() const;

    /**
     * Get segment support geometry data.
     */
    const struct SegmentSupportGeometry* getSegmentSupportGeometry() const;

    /**
     * Get position data for segment support structure.
     * @param[in] plane   Plane number.
     * @param[in] support Segment support element number.
     */
    const struct SegmentSupportPosition*
    getSegmentSupportPosition(int plane, int support) const;

    /**
     * Get strip geometry data.
     */
    const struct StripGeometry* getStripGeometry() const;

    /**
     * Get position data for strips.
     * @param[in] strip Strip number.
     */
    const struct ElementPosition* getStripPosition(int strip) const;

    /**
     * Get shield layer details geometry data.
     */
    const struct ShieldGeometry* getShieldGeometry() const;

    /**
     * Get readout board geometry data.
     */
    const struct BoardGeometry* getBoardGeometry() const;

    /**
     * Get position data for readout boards.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     */
    const struct BoardPosition* getBoardPosition(int plane, int segment) const;

    /**
     * Get position data for strip readout boards.
     * @param[in] board Number of board.
     */
    const struct StripBoardPosition* getStripBoardPosition(int board) const;

  protected:

    /** Detector mode. */
    enum DetectorMode m_Mode;

    /** Number of endcaps. */
    int m_NEndcaps;

    /** Number of layers in one endcap. */
    int m_NLayers;

    /** Number of detector layers. */
    int* m_NDetectorLayers; //[m_NEndcaps]

    /** Number of sectors in one layer. */
    int m_NSectors;

    /** Number of planes in one sector. */
    int m_NPlanes;

    /** Number of segments in one plane. */
    int m_NSegments;

    /** Number of segment support elements in one sector. */
    int m_NSegmentSupportElementsSector;

    /** Number of strips in one segment. */
    int m_NStripsSegment;

    /** Number of strips in one plane. */
    int m_NStrips;

    /** Number of readout boards corresponding to one plane. */
    int m_NBoards;

    /** Number of readout boards in one sector. */
    int m_NBoardsSector;

    /** Number of strip readout boards on one segment readout board. */
    int m_NStripBoards;

    /** Solenoid center Z coordinate. */
    double m_SolenoidZ;

    /** Endcap structure geometry data. */
    struct EndcapStructureGeometry m_EndcapStructureGeometry;

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

    /** Position data for planes. */
    struct ElementPosition m_PlanePosition;

    /** Plastic sheet geometry data. */
    struct PlasticSheetGeometry m_PlasticSheetGeometry;

    /** Segment support geometry data. */
    struct SegmentSupportGeometry m_SegmentSupportGeometry;

    /** Position data for segment support structure. */
    struct SegmentSupportPosition* m_SegmentSupportPosition; //[m_NSegmentSupportElementsSector]

    /** Strip geometry data. */
    struct StripGeometry m_StripGeometry;

    /** Position data for strips. */
    struct ElementPosition* m_StripPosition; //[m_NStrips]

    /** Shield layer details geometry data. */
    struct ShieldGeometry m_ShieldGeometry;

    /** Readout board geometry data. */
    struct BoardGeometry m_BoardGeometry;

    /** Positions of readout boards. */
    struct BoardPosition* m_BoardPosition; //[m_NBoardsSector]

    /** Positions of strip readout boards. */
    struct StripBoardPosition* m_StripBoardPosition; //[m_NStripBoards]

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMGeometry, 1);

  };

}

#endif

