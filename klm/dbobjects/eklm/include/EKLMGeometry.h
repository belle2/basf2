/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* CLHEP headers. */
#include <CLHEP/Geometry/Point3D.h>

namespace Belle2 {

  /**
   * Class to store EKLM geometry data in the database.
   */
  class EKLMGeometry : public TObject {

  public:

    /**
     * Endcap srtucture geometry parameters.
     */
    class EndcapStructureGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      EndcapStructureGeometry();

      /**
       * Destructor.
       */
      ~EndcapStructureGeometry();

      /**
       * Get starting angle of the octagonal endcap structure shape.
       */
      double getPhi() const
      {
        return m_Phi;
      }

      /**
       * Set starting angle of the octagonal endcap structure shape.
       * @param[in] phi Starting angle of the octagonal endcap structure shape.
       */
      void setPhi(double phi)
      {
        m_Phi = phi;
      }

      /**
       * Get number of sides.
       */
      int getNSides() const
      {
        return m_NSides;
      }

      /**
       * Set number of sides.
       * @param[in] nSides number of sides.
       */
      void setNSides(int nSides)
      {
        m_NSides = nSides;
      }

    private:

      /** The starting angle of the octagonal endcap structure shape. */
      double m_Phi = 0;

      /** The number of sides (=8 : octagonal). */
      int m_NSides = 0;

      /** Class version. */
      ClassDef(EndcapStructureGeometry, 1);

    };

    /**
     * Position information for the elements of detector.
     */
    class ElementPosition : public TObject {

    public:

      /**
       * Constructor.
       */
      ElementPosition();

      /**
       * Get inner radius.
       */
      double getInnerR() const
      {
        return m_InnerR;
      }

      /**
       * Set inner radius.
       * @param[in] innerR Inner radius.
       */
      void setInnerR(double innerR)
      {
        m_InnerR = innerR;
      }

      /**
       * Get outer radius.
       */
      double getOuterR() const
      {
        return m_OuterR;
      }

      /**
       * Set outer radius.
       * @param[in] outerR Outer radius.
       */
      void setOuterR(double outerR)
      {
        m_OuterR = outerR;
      }

      /**
       * Get length.
       */
      double getLength() const
      {
        return m_Length;
      }

      /**
       * Set length.
       * @param[in] length Length.
       */
      void setLength(double length)
      {
        m_Length = length;
      }

      /**
       * Get X coordinate.
       */
      double getX() const
      {
        return m_X;
      }

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x)
      {
        m_X = x;
      }

      /**
       * Get Y coordinate.
       */
      double getY() const
      {
        return m_Y;
      }

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y)
      {
        m_Y = y;
      }

      /**
       * Get Z coordinate.
       */
      double getZ() const
      {
        return m_Z;
      }

      /**
       * Set Z coordinate.
       * @param[in] z Z coordinate.
       */
      void setZ(double z)
      {
        m_Z = z;
      }

    private:

      /** Inner radius. */
      double m_InnerR = 0;

      /** Outer radius. */
      double m_OuterR = 0;

      /** Length. */
      double m_Length = 0;

      /** X coordinate. */
      double m_X = 0;

      /** Y coordinate. */
      double m_Y = 0;

      /** Z coordinate. */
      double m_Z = 0;

      /** Class version. */
      ClassDef(ElementPosition, 1);

    };

    /**
     * Sector support geometry data.
     */
    class SectorSupportGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      SectorSupportGeometry();

      /**
       * Get thickness.
       */
      double getThickness() const
      {
        return m_Thickness;
      }

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness)
      {
        m_Thickness = thickness;
      }

      /**
       * Get outerR - Y of upper edge of BoxY.
       */
      double getDeltaLY() const
      {
        return m_DeltaLY;
      }

      /**
       * Set outerR - Y of upper edge of BoxY.
       * @param[in] deltaLY outerR - Y of upper edge of BoxY.
       */
      void setDeltaLY(double deltaLY)
      {
        m_DeltaLY = deltaLY;
      }

      /**
       * Get coordinate X of corner 1.
       */
      double getCornerX() const
      {
        return m_CornerX;
      }

      /**
       * Set coordinate X of corner 1.
       * @param[in] cornerX Coordinate X of corner 1.
       */
      void setCornerX(double cornerX)
      {
        m_CornerX = cornerX;
      }

      /**
       * Get corner 1 X length.
       */
      double getCorner1LX() const
      {
        return m_Corner1LX;
      }

      /**
       * Set corner 1 X length.
       * @param[in] corner1LX Corner 1 X length.
       */
      void setCorner1LX(double corner1LX)
      {
        m_Corner1LX = corner1LX;
      }

      /**
       * Get corner 1 width.
       */
      double getCorner1Width() const
      {
        return m_Corner1Width;
      }

      /**
       * Set corner 1 width.
       * @param[in] corner1Width Corner 1 width.
       */
      void setCorner1Width(double corner1Width)
      {
        m_Corner1Width = corner1Width;
      }

      /**
       * Get corner 1 thickness.
       */
      double getCorner1Thickness() const
      {
        return m_Corner1Thickness;
      }

      /**
       * Set corner 1 thickness.
       * @param[in] corner1Thickness Corner 1 thickness.
       */
      void setCorner1Thickness(double corner1Thickness)
      {
        m_Corner1Thickness = corner1Thickness;
      }

      /**
       * Get corner 1 Z coordinate.
       */
      double getCorner1Z() const
      {
        return m_Corner1Z;
      }

      /**
       * Set corner 1 Z coordinate.
       * @param[in] corner1Z Corner 1 Z coordinate.
       */
      void setCorner1Z(double corner1Z)
      {
        m_Corner1Z = corner1Z;
      }

      /**
       * Get corner 2 X length.
       */
      double getCorner2LX() const
      {
        return m_Corner2LX;
      }

      /**
       * Set corner 2 X length.
       * @param[in] corner2LX Corner 2 X length.
       */
      void setCorner2LX(double corner2LX)
      {
        m_Corner2LX = corner2LX;
      }

      /**
       * Get corner 2 Y length.
       */
      double getCorner2LY() const
      {
        return m_Corner2LY;
      }

      /**
       * Set corner 2 Y length.
       * @param[in] corner2LY Corner 2 Y length.
       */
      void setCorner2LY(double corner2LY)
      {
        m_Corner2LY = corner2LY;
      }

      /**
       * Get corner 2 thickness.
       */
      double getCorner2Thickness() const
      {
        return m_Corner2Thickness;
      }

      /**
       * Set corner 2 thickness.
       * @param[in] corner2Thickness Corner 2 thickness.
       */
      void setCorner2Thickness(double corner2Thickness)
      {
        m_Corner2Thickness = corner2Thickness;
      }

      /**
       * Get corner 2 Z coordinate.
       */
      double getCorner2Z() const
      {
        return m_Corner2Z;
      }

      /**
       * Set corner 2 Z coordinate.
       * @param[in] corner2Z Corner 2 Z coordinate.
       */
      void setCorner2Z(double corner2Z)
      {
        m_Corner2Z = corner2Z;
      }

      /**
       * Get corner 3 X length.
       */
      double getCorner3LX() const
      {
        return m_Corner3LX;
      }

      /**
       * Set corner 3 X length.
       * @param[in] corner3LX Corner 3 X length.
       */
      void setCorner3LX(double corner3LX)
      {
        m_Corner3LX = corner3LX;
      }

      /**
       * Get corner 3 Y length.
       */
      double getCorner3LY() const
      {
        return m_Corner3LY;
      }

      /**
       * Set corner 3 Y length.
       * @param[in] corner3LY Corner 3 Y length.
       */
      void setCorner3LY(double corner3LY)
      {
        m_Corner3LY = corner3LY;
      }

      /**
       * Get corner 3 thickness.
       */
      double getCorner3Thickness() const
      {
        return m_Corner3Thickness;
      }

      /**
       * Set corner 3 thickness.
       * @param[in] corner3Thickness Corner 3 thickness.
       */
      void setCorner3Thickness(double corner3Thickness)
      {
        m_Corner3Thickness = corner3Thickness;
      }

      /**
       * Get corner 3 Z coordinate.
       */
      double getCorner3Z() const
      {
        return m_Corner3Z;
      }

      /**
       * Set corner 3 Z coordinate.
       * @param[in] corner3Z Corner 3 Z coordinate.
       */
      void setCorner3Z(double corner3Z)
      {
        m_Corner3Z = corner3Z;
      }

      /**
       * Get corner 4 X length.
       */
      double getCorner4LX() const
      {
        return m_Corner4LX;
      }

      /**
       * Set corner 4 X length.
       * @param[in] corner4LX Corner 4 X length.
       */
      void setCorner4LX(double corner4LX)
      {
        m_Corner4LX = corner4LX;
      }

      /**
       * Get corner 4 Y length.
       */
      double getCorner4LY() const
      {
        return m_Corner4LY;
      }

      /**
       * Set corner 4 Y length.
       * @param[in] corner4LY Corner 4 Y length.
       */
      void setCorner4LY(double corner4LY)
      {
        m_Corner4LY = corner4LY;
      }

      /**
       * Get corner 4 thickness.
       */
      double getCorner4Thickness() const
      {
        return m_Corner4Thickness;
      }

      /**
       * Set corner 4 thickness.
       * @param[in] corner4Thickness Corner 4 thickness.
       */
      void setCorner4Thickness(double corner4Thickness)
      {
        m_Corner4Thickness = corner4Thickness;
      }

      /**
       * Get corner 4 Z coordinate.
       */
      double getCorner4Z() const
      {
        return m_Corner4Z;
      }

      /**
       * Set corner 4 Z coordinate.
       * @param[in] corner4Z Corner 4 Z coordinate.
       */
      void setCorner4Z(double corner4Z)
      {
        m_Corner4Z = corner4Z;
      }

      /**
       * Get corner 1 angle.
       */
      double getCornerAngle() const
      {
        return m_CornerAngle;
      }

      /**
       * Set corner 1 angle.
       * @param[in] cornerAngle Corner 1 angle.
       */
      void setCornerAngle(double cornerAngle)
      {
        m_CornerAngle = cornerAngle;
      }

      /**
       * Get corner 1A coordinates.
       */
      const HepGeom::Point3D<double>& getCorner1A() const
      {
        return m_Corner1A;
      }

      /**
       * Set corner 1A coordinates.
       * @param[in] corner1A Corner 1A coordinates.
       */
      void setCorner1A(const HepGeom::Point3D<double>& corner1A)
      {
        m_Corner1A = corner1A;
      }

      /**
       * Get corner 1A coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner1AInner() const
      {
        return m_Corner1AInner;
      }

      /**
       * Set corner 1A coordinates (inner side).
       * @param[in] corner1AInner Corner 1A coordinates (inner side).
       */
      void setCorner1AInner(const HepGeom::Point3D<double>& corner1AInner)
      {
        m_Corner1AInner = corner1AInner;
      }

      /**
       * Get corner 1B coordinates.
       */
      const HepGeom::Point3D<double>& getCorner1B() const
      {
        return m_Corner1B;
      }

      /**
       * Set corner 1B coordinates.
       * @param[in] corner1B Corner 1B coordinates.
       */
      void setCorner1B(const HepGeom::Point3D<double>& corner1B)
      {
        m_Corner1B = corner1B;
      }

      /**
       * Get corner 1B coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner1BInner() const
      {
        return m_Corner1BInner;
      }

      /**
       * Set corner 1B coordinates (inner side).
       * @param[in] corner1BInner Corner 1B coordinates (inner side).
       */
      void setCorner1BInner(const HepGeom::Point3D<double>& corner1BInner)
      {
        m_Corner1BInner = corner1BInner;
      }

      /**
       * Get corner 2 coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner2Inner() const
      {
        return m_Corner2Inner;
      }

      /**
       * Set corner 2 coordinates (inner side).
       * @param[in] corner2Inner Corner 2 coordinates (inner side).
       */
      void setCorner2Inner(const HepGeom::Point3D<double>& corner2Inner)
      {
        m_Corner2Inner = corner2Inner;
      }

      /**
       * Get corner 3 coordinates.
       */
      const HepGeom::Point3D<double>& getCorner3() const
      {
        return m_Corner3;
      }

      /**
       * Set corner 3 coordinates.
       * @param[in] corner3 Corner 3 coordinates.
       */
      void setCorner3(const HepGeom::Point3D<double>& corner3)
      {
        m_Corner3 = corner3;
      }

      /**
       * Get corner 3 coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner3Inner() const
      {
        return m_Corner3Inner;
      }

      /**
       * Set corner 3 coordinates (inner side).
       * @param[in] corner3Inner corner 3 coordinates (inner side).
       */
      void setCorner3Inner(const HepGeom::Point3D<double>& corner3Inner)
      {
        m_Corner3Inner = corner3Inner;
      }

      /**
       * Get coordinates of the corner of corner 3 prism.
       */
      const HepGeom::Point3D<double>& getCorner3Prism() const
      {
        return m_Corner3Prism;
      }

      /**
       * Set coordinates of the corner of corner 3 prism.
       * @param[in] corner3Prism Coordinates of the corner of corner 3 prism.
       */
      void setCorner3Prism(const HepGeom::Point3D<double>& corner3Prism)
      {
        m_Corner3Prism = corner3Prism;
      }

      /**
       * Get corner 4 coordinates.
       */
      const HepGeom::Point3D<double>& getCorner4() const
      {
        return m_Corner4;
      }

      /**
       * Set corner 4 coordinates.
       * @param[in] corner4 Corner 4 coordinates.
       */
      void setCorner4(const HepGeom::Point3D<double>& corner4)
      {
        m_Corner4 = corner4;
      }

      /**
       * Get corner 4 coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner4Inner() const
      {
        return m_Corner4Inner;
      }

      /**
       * Set corner 4 coordinates (inner side).
       * @param[in] corner4Inner Corner 4 coordinates (inner side).
       */
      void setCorner4Inner(const HepGeom::Point3D<double>& corner4Inner)
      {
        m_Corner4Inner = corner4Inner;
      }

      /**
       * Get coordinates of the corner of corner 4 prism.
       */
      const HepGeom::Point3D<double>& getCorner4Prism() const
      {
        return m_Corner4Prism;
      }

      /**
       * Set coordinates of the corner of corner 4 prism.
       * @param[in] corner4Prism Coordinates of the corner of corner 4 prism.
       */
      void setCorner4Prism(const HepGeom::Point3D<double>& corner4Prism)
      {
        m_Corner4Prism = corner4Prism;
      }

    private:

      /** Thickness. */
      double m_Thickness = 0;

      /** outerR - Y of upper edge of BoxY. */
      double m_DeltaLY = 0;

      /** Coordinate X of corner 1. */
      double m_CornerX = 0;

      /** Corner 1 X length. */
      double m_Corner1LX = 0;

      /** Corner 1 width. */
      double m_Corner1Width = 0;

      /** Corner 1 thickness. */
      double m_Corner1Thickness = 0;

      /** Corner 1 Z coordinate. */
      double m_Corner1Z = 0;

      /** Corner 2 X length. */
      double m_Corner2LX = 0;

      /** Corner 2 Y length. */
      double m_Corner2LY = 0;

      /** Corner 2 thickness. */
      double m_Corner2Thickness = 0;

      /** Corner 2 Z coordinate. */
      double m_Corner2Z = 0;

      /** Corner 3 X length. */
      double m_Corner3LX = 0;

      /** Corner 3 Y length. */
      double m_Corner3LY = 0;

      /** Corner 3 thickness. */
      double m_Corner3Thickness = 0;

      /** Corner 3 Z coordinate. */
      double m_Corner3Z = 0;

      /** Corner 4 X length. */
      double m_Corner4LX = 0;

      /** Corner 4 Y length. */
      double m_Corner4LY = 0;

      /** Corner 4 thickness. */
      double m_Corner4Thickness = 0;

      /** Corner 4 Z coordinate. */
      double m_Corner4Z = 0;

      /* The following data members are not stored in the database. */

      /** Corner 1 angle. */
      double m_CornerAngle = 0;                 //!

      /** Corner 1A coordinates. */
      HepGeom::Point3D<double> m_Corner1A;      //!

      /** Corner 1A coordinates (inner side). */
      HepGeom::Point3D<double> m_Corner1AInner; //!

      /** Corner 1B coordinates. */
      HepGeom::Point3D<double> m_Corner1B;      //!

      /** Corner 1B coordinates (inner side). */
      HepGeom::Point3D<double> m_Corner1BInner; //!

      /** Corner 2 coordinates (inner side) (corner of prism, too). */
      HepGeom::Point3D<double> m_Corner2Inner;  //!

      /** Corner 3 coordinates. */
      HepGeom::Point3D<double> m_Corner3;       //!

      /** Corner 3 coordinates (inner side). */
      HepGeom::Point3D<double> m_Corner3Inner;  //!

      /** Coordinates of the corner of corner 3 prism. */
      HepGeom::Point3D<double> m_Corner3Prism;  //!

      /** Corner 4 coordinates. */
      HepGeom::Point3D<double> m_Corner4;       //!

      /** Corner 4 coordinates (inner side). */
      HepGeom::Point3D<double> m_Corner4Inner;  //!

      /** Coordinates of the corner of corner 4 prism. */
      HepGeom::Point3D<double> m_Corner4Prism;  //!

      /** Class version. */
      ClassDef(SectorSupportGeometry, 1);

    };

    /**
     * Plastic sheet geometry data.
     */
    class PlasticSheetGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      PlasticSheetGeometry();

      /**
       * Get width.
       */
      double getWidth() const
      {
        return m_Width;
      }

      /**
       * Set width.
       * @param[in] width Width.
       */
      void setWidth(double width)
      {
        m_Width = width;
      }

      /**
       * Get Delta L (edge of last strip - edge of plastic sheet distance).
       */
      double getDeltaL() const
      {
        return m_DeltaL;
      }

      /**
       * Set Delta L (edge of last strip - edge of plastic sheet distance).
       * @param[in] deltaL DeltaL.
       */
      void setDeltaL(double deltaL)
      {
        m_DeltaL = deltaL;
      }

    private:

      /** Width. */
      double m_Width = 0;

      /** Distance from edge of last strip to edge of plastic sheet. */
      double m_DeltaL = 0;

      /** Class version. */
      ClassDef(PlasticSheetGeometry, 1);

    };

    /**
     * Segment support geometry data.
     */
    class SegmentSupportGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      SegmentSupportGeometry();

      /**
       * Get top part width.
       */
      double getTopWidth() const
      {
        return m_TopWidth;
      }

      /**
       * Set top part width.
       * @param[in] topWidth Top part width.
       */
      void setTopWidth(double topWidth)
      {
        m_TopWidth = topWidth;
      }

      /**
       * Get top part thickness.
       */
      double getTopThickness() const
      {
        return m_TopThickness;
      }

      /**
       * Set top part thickness.
       * @param[in] topThickness Top part thickness.
       */
      void setTopThickness(double topThickness)
      {
        m_TopThickness = topThickness;
      }

      /**
       * Get middle part width.
       */
      double getMiddleWidth() const
      {
        return m_MiddleWidth;
      }

      /**
       * Set middle part width.
       * @param[in] middleWidth Middle part width.
       */
      void setMiddleWidth(double middleWidth)
      {
        m_MiddleWidth = middleWidth;
      }

      /**
       * Get middle part thickness.
       */
      double getMiddleThickness() const
      {
        return m_MiddleThickness;
      }

      /**
       * Set middle part thickness.
       * @param[in] middleThickness Middle part thickness.
       */
      void setMiddleThickness(double middleThickness)
      {
        m_MiddleThickness = middleThickness;
      }

    private:

      /** Top box width. */
      double m_TopWidth = 0;

      /** Top box thickness. */
      double m_TopThickness = 0;

      /** Middle box width. */
      double m_MiddleWidth = 0;

      /** Middle box thickness. */
      double m_MiddleThickness = 0;

      /** Class version. */
      ClassDef(SegmentSupportGeometry, 1);

    };

    /**
     * Segment support position.
     */
    class SegmentSupportPosition : public TObject {

    public:

      /**
       * Constructor.
       */
      SegmentSupportPosition();

      /**
       * Get right Delta L.
       */
      double getDeltaLRight() const
      {
        return m_DeltaLRight;
      }

      /**
       * Set right Delta L.
       * @param[in] deltaLRight Right Delta L.
       */
      void setDeltaLRight(double deltaLRight)
      {
        m_DeltaLRight = deltaLRight;
      }

      /**
       * Get left Delta L.
       */
      double getDeltaLLeft() const
      {
        return m_DeltaLLeft;
      }

      /**
       * Set left DeltaL.
       * @param[in] deltaLLeft Left Delta L.
       */
      void setDeltaLLeft(double deltaLLeft)
      {
        m_DeltaLLeft = deltaLLeft;
      }

      /**
       * Get length.
       */
      double getLength() const
      {
        return m_Length;
      }

      /**
       * Set length.
       * @param[in] length Length.
       */
      void setLength(double length)
      {
        m_Length = length;
      }

      /**
       * Get X coordinate.
       */
      double getX() const
      {
        return m_X;
      }

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x)
      {
        m_X = x;
      }

      /**
       * Get Y coordinate.
       */
      double getY() const
      {
        return m_Y;
      }

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y)
      {
        m_Y = y;
      }

      /**
       * Get Z coordinate.
       */
      double getZ() const
      {
        return m_Z;
      }

      /**
       * Set Z coordinate.
       * @param[in] z Z coordinate.
       */
      void setZ(double z)
      {
        m_Z = z;
      }

    private:

      /** Right (X-plane) delta L. */
      double m_DeltaLRight = 0;

      /** Left (X-plane) delta L. */
      double m_DeltaLLeft = 0;

      /** Length */
      double m_Length = 0;

      /** X coordinate. */
      double m_X = 0;

      /** Y coordinate. */
      double m_Y = 0;

      /** Z coordinate. */
      double m_Z = 0;

      /** Class version. */
      ClassDef(SegmentSupportPosition, 1);

    };

    /**
     * Strip geometry data.
     */
    class StripGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      StripGeometry();

      /**
       * Get width.
       */
      double getWidth() const
      {
        return m_Width;
      }

      /**
       * Set width.
       * @param[in] width Width.
       */
      void setWidth(double width)
      {
        m_Width = width;
      }

      /**
       * Get thickness.
       */
      double getThickness() const
      {
        return m_Thickness;
      }

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness)
      {
        m_Thickness = thickness;
      }

      /**
       * Get groove depth.
       */
      double getGrooveDepth() const
      {
        return m_GrooveDepth;
      }

      /**
       * Set groove depth.
       * @param[in] grooveDepth Groove depth.
       */
      void setGrooveDepth(double grooveDepth)
      {
        m_GrooveDepth = grooveDepth;
      }

      /**
       * Get groove width.
       */
      double getGrooveWidth() const
      {
        return m_GrooveWidth;
      }

      /**
       * Set groove width.
       * @param[in] grooveWidth Groove width.
       */
      void setGrooveWidth(double grooveWidth)
      {
        m_GrooveWidth = grooveWidth;
      }

      /**
       * Get nonscintillating layer thickness..
       */
      double getNoScintillationThickness() const
      {
        return m_NoScintillationThickness;
      }

      /**
       * Set nonscintillating layer thickness..
       * @param[in] thickness Nonscintillating layer thickness.
       */
      void setNoScintillationThickness(double thickness)
      {
        m_NoScintillationThickness = thickness;
      }

      /**
       * Get "SiPM" size.
       */
      double getRSSSize() const
      {
        return m_RSSSize;
      }

      /**
       * Set "SiPM" size.
       * @param[in]
       */
      void setRSSSize(double rssSize)
      {
        m_RSSSize = rssSize;
      }

    private:

      /** Width. */
      double m_Width = 0;

      /** Thickness. */
      double m_Thickness = 0;

      /** Groove depth. */
      double m_GrooveDepth = 0;

      /** Groove width. */
      double m_GrooveWidth = 0;

      /** Non-scintillating layer thickness. */
      double m_NoScintillationThickness = 0;

      /** Radiation study SiPM size. */
      double m_RSSSize = 0;

      /** Class version. */
      ClassDef(StripGeometry, 1);

    };

    /**
     * 2D point.
     */
    class Point : public TObject {

    public:

      /**
       * Constructor.
       */
      Point();

      /**
       * Get X coordinate.
       */
      double getX() const
      {
        return m_X;
      }

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x)
      {
        m_X = x;
      }

      /**
       * Get Y coordinate.
       */
      double getY() const
      {
        return m_Y;
      }

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y)
      {
        m_Y = y;
      }

    private:

      /** X coordinate. */
      double m_X = 0;

      /** Y coordinate. */
      double m_Y = 0;

      /** Class version. */
      ClassDef(Point, 1);

    };

    /**
     * Shield layer detail geometry data.
     */
    class ShieldDetailGeometry : public TObject {

    public:

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

      /**
       * Destructor.
       */
      ~ShieldDetailGeometry();

      /**
       * Get X length.
       */
      double getLengthX() const
      {
        return m_LengthX;
      }

      /**
       * Set X length.
       * @param[in] lengthX X length.
       */
      void setLengthX(double lengthX)
      {
        m_LengthX = lengthX;
      }

      /**
       * Get Y length.
       */
      double getLengthY() const
      {
        return m_LengthY;
      }

      /**
       * Set Y length.
       * @param[in] lengthY Y length.
       */
      void setLengthY(double lengthY)
      {
        m_LengthY = lengthY;
      }

      /**
       * Get number of points.
       */
      int getNPoints() const
      {
        return m_NPoints;
      }

      /**
       * Set number of points.
       * @param[in] nPoints Number of points.
       */
      void setNPoints(int nPoints);

      /**
       * Get point.
       * @param[in] i Number of point (array index).
       */
      const Point* getPoint(int i) const;

      /**
       * Set point.
       * @param[in] i     Number of point (array index).
       * @param[in] point Point.
       */
      void setPoint(int i, const Point& point);

    private:

      /** X length. */
      double m_LengthX = 0;

      /** Y length. */
      double m_LengthY = 0;

      /** Number of points. */
      int m_NPoints = 0;

      /** Points. */
      Point* m_Points = nullptr; //[m_NPoints]

      /** Class version. */
      ClassDef(ShieldDetailGeometry, 1);

    };

    /**
     * Shield layer geometry data.
     */
    class ShieldGeometry : public TObject {

    public:

      /**
       * Constructor.
       */
      ShieldGeometry();

      /**
       * Get thickness.
       */
      double getThickness() const
      {
        return m_Thickness;
      }

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness)
      {
        m_Thickness = thickness;
      }

      /**
       * Get detail A geometry.
       */
      const ShieldDetailGeometry* getDetailA() const
      {
        return &m_DetailA;
      }

      /**
       * Set detail A geometry.
       * @param[in] geometry Detail A geometry.
       */
      void setDetailA(const ShieldDetailGeometry& geometry)
      {
        m_DetailA = geometry;
      }

      /**
       * Get detail B geometry.
       */
      const ShieldDetailGeometry* getDetailB() const
      {
        return &m_DetailB;
      }

      /**
       * Set detail B geometry.
       * @param[in] geometry Detail B geometry.
       */
      void setDetailB(const ShieldDetailGeometry& geometry)
      {
        m_DetailB = geometry;
      }

      /**
       * Get detail C geometry.
       */
      const ShieldDetailGeometry* getDetailC() const
      {
        return &m_DetailC;
      }

      /**
       * Set detail C geometry.
       * @param[in] geometry Detail C geometry.
       */
      void setDetailC(const ShieldDetailGeometry& geometry)
      {
        m_DetailC = geometry;
      }

      /**
       * Get detail D geometry.
       */
      const ShieldDetailGeometry* getDetailD() const
      {
        return &m_DetailD;
      }

      /**
       * Set detail D geometry.
       * @param[in] geometry Detail D geometry.
       */
      void setDetailD(const ShieldDetailGeometry& geometry)
      {
        m_DetailD = geometry;
      }

      /**
       * Get detail A center.
       */
      const Point* getDetailACenter() const
      {
        return &m_DetailACenter;
      }

      /**
       * Set detail A center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailACenter(double x, double y);

      /**
       * Get detail B center.
       */
      const Point* getDetailBCenter() const
      {
        return &m_DetailBCenter;
      }

      /**
       * Set detail B center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailBCenter(double x, double y);

      /**
       * Get detail C center.
       */
      const Point* getDetailCCenter() const
      {
        return &m_DetailCCenter;
      }

      /**
       * Set detail C center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailCCenter(double x, double y);

    private:

      /** Thickness. */
      double m_Thickness = 0;

      /** Detail A. */
      ShieldDetailGeometry m_DetailA;

      /** Detail B. */
      ShieldDetailGeometry m_DetailB;

      /** Detail C. */
      ShieldDetailGeometry m_DetailC;

      /** Detail D. */
      ShieldDetailGeometry m_DetailD;

      /* The following data members are not stored in the database. */

      /** Detail A center. */
      Point m_DetailACenter; //!

      /** Detail B center. */
      Point m_DetailBCenter; //!

      /** Detail C center. */
      Point m_DetailCCenter; //!

      /** Class version. */
      ClassDef(ShieldGeometry, 1);

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
     * Get number of sections.
     */
    int getNSections() const
    {
      return m_NSections;
    }

    /**
     * Get number of layers.
     */
    int getNLayers() const
    {
      return m_NLayers;
    }

    /**
     * Get number of detector layers.
     * @param[in] section Section number (1 - backward, 2 - forward).
     */
    int getNDetectorLayers(int section) const;

    /**
     * Get number of sectors.
     */
    int getNSectors() const
    {
      return m_NSectors;
    }

    /**
     * Get number of planes.
     */
    int getNPlanes() const
    {
      return m_NPlanes;
    }

    /**
     * Get number of segments.
     */
    int getNSegments() const
    {
      return m_NSegments;
    }

    /**
     * Get number of segment support elements *in one sector).
     */
    int getNSegmentSupportElementsSector() const
    {
      return m_NSegmentSupportElementsSector;
    }

    /**
     * Get number of strips.
     */
    int getNStrips() const
    {
      return m_NStrips;
    }

    /**
     * Check if number of detector layers is correct (fatal error if not).
     * Section number must be checked separately.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     */
    void checkDetectorLayerNumber(int section, int layer) const;

    /**
     * Check if detector layer number is correct (fatal error if not).
     * Section number must be checked separately.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     */
    void checkDetectorLayer(int section, int layer) const;

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
    double getSolenoidZ() const
    {
      return m_SolenoidZ;
    }

    /**
     * Get endcap structure geometry data.
     */
    const EndcapStructureGeometry* getEndcapStructureGeometry() const
    {
      return &m_EndcapStructureGeometry;
    }

    /**
     * Get position data for sections.
     */
    const ElementPosition* getSectionPosition() const
    {
      return &m_SectionPosition;
    }

    /**
     * Get position data for layers.
     */
    const ElementPosition* getLayerPosition() const
    {
      return &m_LayerPosition;
    }

    /**
     * Get Z distance between two layers.
     */
    double getLayerShiftZ() const
    {
      return m_LayerShiftZ;
    }

    /**
     * Get position data for sectors.
     */
    const ElementPosition* getSectorPosition() const
    {
      return &m_SectorPosition;
    }

    /**
     * Get position data for sector support structure.
     */
    const ElementPosition* getSectorSupportPosition() const
    {
      return &m_SectorSupportPosition;
    }

    /**
     * Get sector support geometry data.
     */
    const SectorSupportGeometry* getSectorSupportGeometry() const
    {
      return &m_SectorSupportGeometry;
    }

    /**
     * Get position data for planes.
     */
    const ElementPosition* getPlanePosition() const
    {
      return &m_PlanePosition;
    }

    /**
     * Get plastic sheet geometry data.
     */
    const PlasticSheetGeometry* getPlasticSheetGeometry() const
    {
      return &m_PlasticSheetGeometry;
    }

    /**
     * Get segment support geometry data.
     */
    const SegmentSupportGeometry* getSegmentSupportGeometry() const
    {
      return &m_SegmentSupportGeometry;
    }

    /**
     * Get position data for segment support structure.
     * @param[in] plane   Plane number.
     * @param[in] support Segment support element number.
     */
    const SegmentSupportPosition*
    getSegmentSupportPosition(int plane, int support) const;

    /**
     * Get strip geometry data.
     */
    const StripGeometry* getStripGeometry() const
    {
      return &m_StripGeometry;
    }

    /**
     * Get position data for strips.
     * @param[in] strip Strip number.
     */
    const ElementPosition* getStripPosition(int strip) const;

    /**
     * Get shield layer details geometry data.
     */
    const ShieldGeometry* getShieldGeometry() const
    {
      return &m_ShieldGeometry;
    }

  protected:

    /** Element numbers. */
    const EKLMElementNumbers* m_ElementNumbers; //! ROOT streamer

    /** Number of sections. */
    int m_NSections;

    /** Number of layers in one section. */
    int m_NLayers;

    /** Number of detector layers. */
    int* m_NDetectorLayers; //[m_NSections]

    /** Number of sectors in one layer. */
    int m_NSectors;

    /** Number of planes in one sector. */
    int m_NPlanes;

    /** Number of segments in one plane. */
    int m_NSegments;

    /** Number of segment support elements in one sector. */
    int m_NSegmentSupportElementsSector;

    /** Number of strips in one plane. */
    int m_NStrips;

    /** Solenoid center Z coordinate. */
    double m_SolenoidZ;

    /** Section structure geometry data. */
    EndcapStructureGeometry m_EndcapStructureGeometry;

    /** Position data for sections. */
    ElementPosition m_SectionPosition;

    /** Position data for layers. */
    ElementPosition m_LayerPosition;

    /** Z distance between two layers. */
    double m_LayerShiftZ;

    /** Position data for sectors. */
    ElementPosition m_SectorPosition;

    /** Position data for sector support structure. */
    ElementPosition m_SectorSupportPosition;

    /** Sector support geometry data. */
    SectorSupportGeometry m_SectorSupportGeometry;

    /** Position data for planes. */
    ElementPosition m_PlanePosition;

    /** Plastic sheet geometry data. */
    PlasticSheetGeometry m_PlasticSheetGeometry;

    /** Segment support geometry data. */
    SegmentSupportGeometry m_SegmentSupportGeometry;

    /** Position data for segment support structure. */
    SegmentSupportPosition* m_SegmentSupportPosition; //[m_NSegmentSupportElementsSector]

    /** Strip geometry data. */
    StripGeometry m_StripGeometry;

    /** Position data for strips. */
    ElementPosition* m_StripPosition; //[m_NStrips]

    /** Shield layer details geometry data. */
    ShieldGeometry m_ShieldGeometry;

    /** Class version. */
    ClassDef(Belle2::EKLMGeometry, 5);

  };

}
