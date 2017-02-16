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

    /**
     * EndcapStructure geometry parameters.
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
       * Get starting angle of the octagonal Endcap KLM shape.
       */
      double getPhi() const;

      /**
       * Set starting angle of the octagonal Endcap KLM shape.
       * @param[in] phi Starting angle of the octagonal Endcap KLM shape.
       */
      void setPhi(double phi);

      /**
       * Get number of sides.
       */
      int getNSides() const;

      /**
       * Set number of sides.
       * @param[in] nSides number of sides.
       */
      void setNSides(int nSides);

    private:

      /** The starting angle of the octagonal Endcap KLM shape. */
      double m_Phi;

      /** The number of sides (=8 : octagonal). */
      int m_NSides;

      /** Makes objects storable. */
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
      double getInnerR() const;

      /**
       * Set inner radius.
       * @param[in] innerR Inner radius.
       */
      void setInnerR(double innerR);

      /**
       * Get outer radius.
       */
      double getOuterR() const;

      /**
       * Set outer radius.
       * @param[in] outerR Outer radius.
       */
      void setOuterR(double outerR);

      /**
       * Get length.
       */
      double getLength() const;

      /**
       * Set length.
       * @param[in] length Length.
       */
      void setLength(double length);

      /**
       * Get X coordinate.
       */
      double getX() const;

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x);

      /**
       * Get Y coordinate.
       */
      double getY() const;

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y);

      /**
       * Get Z coordinate.
       */
      double getZ() const;

      /**
       * Set Z coordinate.
       * @param[in] z Z coordinate.
       */
      void setZ(double z);

    private:

      /** Inner radius. */
      double m_InnerR;

      /** Outer radius. */
      double m_OuterR;

      /** Length. */
      double m_Length;

      /** X coordinate. */
      double m_X;

      /** Y coordinate. */
      double m_Y;

      /** Z coordinate. */
      double m_Z;

      /** Makes objects storable. */
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
      double getThickness() const;

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness);

      /**
       * Get outerR - Y of upper edge of BoxY.
       */
      double getDeltaLY() const;

      /**
       * Set outerR - Y of upper edge of BoxY.
       * @param[in] deltaLY outerR - Y of upper edge of BoxY.
       */
      void setDeltaLY(double deltaLY);

      /**
       * Get coordinate X of corner 1.
       */
      double getCornerX() const;

      /**
       * Set coordinate X of corner 1.
       * @param[in] cornerX Coordinate X of corner 1.
       */
      void setCornerX(double cornerX);

      /**
       * Get corner 1 X length.
       */
      double getCorner1LX() const;

      /**
       * Set corner 1 X length.
       * @param[in] corner1LX Corner 1 X length.
       */
      void setCorner1LX(double corner1LX);

      /**
       * Get corner 1 width.
       */
      double getCorner1Width() const;

      /**
       * Set corner 1 width.
       * @param[in] corner1Width Corner 1 width.
       */
      void setCorner1Width(double corner1Width);

      /**
       * Get corner 1 thickness.
       */
      double getCorner1Thickness() const;

      /**
       * Set corner 1 thickness.
       * @param[in] corner1Thickness Corner 1 thickness.
       */
      void setCorner1Thickness(double corner1Thickness);

      /**
       * Get corner 1 Z coordinate.
       */
      double getCorner1Z() const;

      /**
       * Set corner 1 Z coordinate.
       * @param[in] corner1Z Corner 1 Z coordinate.
       */
      void setCorner1Z(double corner1Z);

      /**
       * Get corner 2 X length.
       */
      double getCorner2LX() const;

      /**
       * Set corner 2 X length.
       * @param[in] corner2LX Corner 2 X length.
       */
      void setCorner2LX(double corner2LX);

      /**
       * Get corner 2 Y length.
       */
      double getCorner2LY() const;

      /**
       * Set corner 2 Y length.
       * @param[in] corner2LY Corner 2 Y length.
       */
      void setCorner2LY(double corner2LY);

      /**
       * Get corner 2 thickness.
       */
      double getCorner2Thickness() const;

      /**
       * Set corner 2 thickness.
       * @param[in] corner2Thickness Corner 2 thickness.
       */
      void setCorner2Thickness(double corner2Thickness);

      /**
       * Get corner 2 Z coordinate.
       */
      double getCorner2Z() const;

      /**
       * Set corner 2 Z coordinate.
       * @param[in] corner2Z Corner 2 Z coordinate.
       */
      void setCorner2Z(double corner2Z);

      /**
       * Get corner 3 X length.
       */
      double getCorner3LX() const;

      /**
       * Set corner 3 X length.
       * @param[in] corner3LX Corner 3 X length.
       */
      void setCorner3LX(double corner3LX);

      /**
       * Get corner 3 Y length.
       */
      double getCorner3LY() const;

      /**
       * Set corner 3 Y length.
       * @param[in] corner3LY Corner 3 Y length.
       */
      void setCorner3LY(double corner3LY);

      /**
       * Get corner 3 thickness.
       */
      double getCorner3Thickness() const;

      /**
       * Set corner 3 thickness.
       * @param[in] corner3Thickness Corner 3 thickness.
       */
      void setCorner3Thickness(double corner3Thickness);

      /**
       * Get corner 3 Z coordinate.
       */
      double getCorner3Z() const;

      /**
       * Set corner 3 Z coordinate.
       * @param[in] corner3Z Corner 3 Z coordinate.
       */
      void setCorner3Z(double corner3Z);

      /**
       * Get corner 4 X length.
       */
      double getCorner4LX() const;

      /**
       * Set corner 4 X length.
       * @param[in] corner4LX Corner 4 X length.
       */
      void setCorner4LX(double corner4LX);

      /**
       * Get corner 4 Y length.
       */
      double getCorner4LY() const;

      /**
       * Set corner 4 Y length.
       * @param[in] corner4LY Corner 4 Y length.
       */
      void setCorner4LY(double corner4LY);

      /**
       * Get corner 4 thickness.
       */
      double getCorner4Thickness() const;

      /**
       * Set corner 4 thickness.
       * @param[in] corner4Thickness Corner 4 thickness.
       */
      void setCorner4Thickness(double corner4Thickness);

      /**
       * Get corner 4 Z coordinate.
       */
      double getCorner4Z() const;

      /**
       * Set corner 4 Z coordinate.
       * @param[in] corner4Z Corner 4 Z coordinate.
       */
      void setCorner4Z(double corner4Z);

      /**
       * Get corner 1 angle.
       */
      double getCornerAngle() const;

      /**
       * Set corner 1 angle.
       * @param[in] cornerAngle Corner 1 angle.
       */
      void setCornerAngle(double cornerAngle);

      /**
       * Get corner 1A coordinates.
       */
      const HepGeom::Point3D<double>& getCorner1A() const;

      /**
       * Set corner 1A coordinates.
       * @param[in] corner1A Corner 1A coordinates.
       */
      void setCorner1A(const HepGeom::Point3D<double>& corner1A);

      /**
       * Get corner 1A coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner1AInner() const;

      /**
       * Set corner 1A coordinates (inner side).
       * @param[in] corner1AInner Corner 1A coordinates (inner side).
       */
      void setCorner1AInner(const HepGeom::Point3D<double>& corner1AInner);

      /**
       * Get corner 1B coordinates.
       */
      const HepGeom::Point3D<double>& getCorner1B() const;

      /**
       * Set corner 1B coordinates.
       * @param[in] corner1B Corner 1B coordinates.
       */
      void setCorner1B(const HepGeom::Point3D<double>& corner1B);

      /**
       * Get corner 1B coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner1BInner() const;

      /**
       * Set corner 1B coordinates (inner side).
       * @param[in] corner1BInner Corner 1B coordinates (inner side).
       */
      void setCorner1BInner(const HepGeom::Point3D<double>& corner1BInner);

      /**
       * Get corner 2 coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner2Inner() const;

      /**
       * Set corner 2 coordinates (inner side).
       * @param[in] corner2Inner Corner 2 coordinates (inner side).
       */
      void setCorner2Inner(const HepGeom::Point3D<double>& corner2Inner);

      /**
       * Get corner 3 coordinates.
       */
      const HepGeom::Point3D<double>& getCorner3() const;

      /**
       * Set corner 3 coordinates.
       * @param[in] corner3 Corner 3 coordinates.
       */
      void setCorner3(const HepGeom::Point3D<double>& corner3);

      /**
       * Get corner 3 coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner3Inner() const;

      /**
       * Set corner 3 coordinates (inner side).
       * @param[in] corner3Inner corner 3 coordinates (inner side).
       */
      void setCorner3Inner(const HepGeom::Point3D<double>& corner3Inner);

      /**
       * Get coordinates of the corner of corner 3 prism.
       */
      const HepGeom::Point3D<double>& getCorner3Prism() const;

      /**
       * Set coordinates of the corner of corner 3 prism.
       * @param[in] corner3Prism Coordinates of the corner of corner 3 prism.
       */
      void setCorner3Prism(const HepGeom::Point3D<double>& corner3Prism);

      /**
       * Get corner 4 coordinates.
       */
      const HepGeom::Point3D<double>& getCorner4() const;

      /**
       * Set corner 4 coordinates.
       * @param[in] corner4 Corner 4 coordinates.
       */
      void setCorner4(const HepGeom::Point3D<double>& corner4);

      /**
       * Get corner 4 coordinates (inner side).
       */
      const HepGeom::Point3D<double>& getCorner4Inner() const;

      /**
       * Set corner 4 coordinates (inner side).
       * @param[in] corner4Inner Corner 4 coordinates (inner side).
       */
      void setCorner4Inner(const HepGeom::Point3D<double>& corner4Inner);

      /**
       * Get coordinates of the corner of corner 4 prism.
       */
      const HepGeom::Point3D<double>& getCorner4Prism() const;

      /**
       * Set coordinates of the corner of corner 4 prism.
       * @param[in] corner4Prism Coordinates of the corner of corner 4 prism.
       */
      void setCorner4Prism(const HepGeom::Point3D<double>& corner4Prism);

    private:

      /** Thickness. */
      double m_Thickness;

      /** outerR - Y of upper edge of BoxY. */
      double m_DeltaLY;

      /** Coordinate X of corner 1. */
      double m_CornerX;

      /** Corner 1 X length. */
      double m_Corner1LX;

      /** Corner 1 width. */
      double m_Corner1Width;

      /** Corner 1 thickness. */
      double m_Corner1Thickness;

      /** Corner 1 Z coordinate. */
      double m_Corner1Z;

      /** Corner 2 X length. */
      double m_Corner2LX;

      /** Corner 2 Y length. */
      double m_Corner2LY;

      /** Corner 2 thickness. */
      double m_Corner2Thickness;

      /** Corner 2 Z coordinate. */
      double m_Corner2Z;

      /** Corner 3 X length. */
      double m_Corner3LX;

      /** Corner 3 Y length. */
      double m_Corner3LY;

      /** Corner 3 thickness. */
      double m_Corner3Thickness;

      /** Corner 3 Z coordinate. */
      double m_Corner3Z;

      /** Corner 4 X length. */
      double m_Corner4LX;

      /** Corner 4 Y length. */
      double m_Corner4LY;

      /** Corner 4 thickness. */
      double m_Corner4Thickness;

      /** Corner 4 Z coordinate. */
      double m_Corner4Z;

      /* The following data members are not stored in the database. */

      /** Corner 1 angle. */
      double m_CornerAngle;                     //!

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

      /** Makes objects storable. */
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
      double getWidth() const;

      /**
       * Set width.
       * @param[in] width Width.
       */
      void setWidth(double width);

      /**
       * Get Delta L (edge of last strip - edge of plastic sheet distance).
       */
      double getDeltaL() const;

      /**
       * Set Delta L (edge of last strip - edge of plastic sheet distance).
       * @param[in] deltaL DeltaL.
       */
      void setDeltaL(double deltaL);

    private:

      /** Width. */
      double m_Width;

      /** Distance from edge of last strip to edge of plastic sheet. */
      double m_DeltaL;

      /** Makes objects storable. */
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
      double getTopWidth() const;

      /**
       * Set top part width.
       * @param[in] topWidth Top part width.
       */
      void setTopWidth(double topWidth);

      /**
       * Get top part thickness.
       */
      double getTopThickness() const;

      /**
       * Set top part thickness.
       * @param[in] topThickness Top part thickness.
       */
      void setTopThickness(double topThickness);

      /**
       * Get middle part width.
       */
      double getMiddleWidth() const;

      /**
       * Set middle part width.
       * @param[in] middleWidth Middle part width.
       */
      void setMiddleWidth(double middleWidth);

      /**
       * Get middle part thickness.
       */
      double getMiddleThickness() const;

      /**
       * Set middle part thickness.
       * @param[in] middleThickness Middle part thickness.
       */
      void setMiddleThickness(double middleThickness);

    private:

      /** Top box width. */
      double m_TopWidth;

      /** Top box thickness. */
      double m_TopThickness;

      /** Middle box width. */
      double m_MiddleWidth;

      /** Middle box thickness. */
      double m_MiddleThickness;

      /** Makes objects storable. */
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
      double getDeltaLRight() const;

      /**
       * Set right Delta L.
       * @param[in] deltaLRight Right Delta L.
       */
      void setDeltaLRight(double deltaLRight);

      /**
       * Get left Delta L.
       */
      double getDeltaLLeft() const;

      /**
       * Set left DeltaL.
       * @param[in] deltaLLeft Left Delta L.
       */
      void setDeltaLLeft(double deltaLLeft);

      /**
       * Get length.
       */
      double getLength() const;

      /**
       * Set length.
       * @param[in] length Length.
       */
      void setLength(double length);

      /**
       * Get X coordinate.
       */
      double getX() const;

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x);

      /**
       * Get Y coordinate.
       */
      double getY() const;

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y);

      /**
       * Get Z coordinate.
       */
      double getZ() const;

      /**
       * Set Z coordinate.
       * @param[in] z Z coordinate.
       */
      void setZ(double z);

    private:

      /** Right (X-plane) delta L. */
      double m_DeltaLRight;

      /** Left (X-plane) delta L. */
      double m_DeltaLLeft;

      /** Length */
      double m_Length;

      /** X coordinate. */
      double m_X;

      /** Y coordinate. */
      double m_Y;

      /** Z coordinate. */
      double m_Z;

      /** Makes objects storable. */
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
      double getWidth() const;

      /**
       * Set width.
       * @param[in] width Width.
       */
      void setWidth(double width);

      /**
       * Get thickness.
       */
      double getThickness() const;

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness);

      /**
       * Get groove depth.
       */
      double getGrooveDepth() const;

      /**
       * Set groove depth.
       * @param[in] grooveDepth Groove depth.
       */
      void setGrooveDepth(double grooveDepth);

      /**
       * Get groove width.
       */
      double getGrooveWidth() const;

      /**
       * Set groove width.
       * @param[in] grooveWidth Groove width.
       */
      void setGrooveWidth(double grooveWidth);

      /**
       * Get nonscintillating layer thickness..
       */
      double getNoScintillationThickness() const;

      /**
       * Set nonscintillating layer thickness..
       * @param[in] thickness Nonscintillating layer thickness.
       */
      void setNoScintillationThickness(double thickness);

      /**
       * Get "SiPM" size.
       */
      double getRSSSize() const;

      /**
       * Set "SiPM" size.
       * @param[in]
       */
      void setRSSSize(double rssSize);

    private:

      /** Width. */
      double m_Width;

      /** Thickness. */
      double m_Thickness;

      /** Groove depth. */
      double m_GrooveDepth;

      /** Groove width. */
      double m_GrooveWidth;

      /** Non-scintillating layer thickness. */
      double m_NoScintillationThickness;

      /** Radiation study SiPM size. */
      double m_RSSSize;

      /** Makes objects storable. */
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
      double getX() const;

      /**
       * Set X coordinate.
       * @param[in] x X coordinate.
       */
      void setX(double x);

      /**
       * Get Y coordinate.
       */
      double getY() const;

      /**
       * Set Y coordinate.
       * @param[in] y Y coordinate.
       */
      void setY(double y);

    private:

      /** X coordinate. */
      double m_X;

      /** Y coordinate. */
      double m_Y;

      /** Makes objects storable. */
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
      double getLengthX() const;

      /**
       * Set X length.
       * @param[in] lengthX X length.
       */
      void setLengthX(double lengthX);

      /**
       * Get Y length.
       */
      double getLengthY() const;

      /**
       * Set Y length.
       * @param[in] lengthY Y length.
       */
      void setLengthY(double lengthY);

      /**
       * Get number of points.
       */
      int getNPoints() const;

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
      double m_LengthX;

      /** Y length. */
      double m_LengthY;

      /** Number of points. */
      int m_NPoints;

      /** Points. */
      Point* m_Points; //[m_NPoints]

      /** Makes objects storable. */
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
      double getThickness() const;

      /**
       * Set thickness.
       * @param[in] thickness Thickness.
       */
      void setThickness(double thickness);

      /**
       * Get detail A geometry.
       */
      const ShieldDetailGeometry* getDetailA() const;

      /**
       * Set detail A geometry.
       * @param[in] geometry Detail A geometry.
       */
      void setDetailA(const ShieldDetailGeometry& geometry);

      /**
       * Get detail B geometry.
       */
      const ShieldDetailGeometry* getDetailB() const;

      /**
       * Set detail B geometry.
       * @param[in] geometry Detail B geometry.
       */
      void setDetailB(const ShieldDetailGeometry& geometry);

      /**
       * Get detail C geometry.
       */
      const ShieldDetailGeometry* getDetailC() const;

      /**
       * Set detail C geometry.
       * @param[in] geometry Detail C geometry.
       */
      void setDetailC(const ShieldDetailGeometry& geometry);

      /**
       * Get detail D geometry.
       */
      const ShieldDetailGeometry* getDetailD() const;

      /**
       * Set detail D geometry.
       * @param[in] geometry Detail D geometry.
       */
      void setDetailD(const ShieldDetailGeometry& geometry);

      /**
       * Get detail A center.
       */
      const Point* getDetailACenter() const;

      /**
       * Set detail A center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailACenter(double x, double y);

      /**
       * Get detail B center.
       */
      const Point* getDetailBCenter() const;

      /**
       * Set detail B center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailBCenter(double x, double y);

      /**
       * Get detail C center.
       */
      const Point* getDetailCCenter() const;

      /**
       * Set detail C center.
       * @param[in] x X coordinate.
       * @param[in] y Y coordinate.
       */
      void setDetailCCenter(double x, double y);

    private:

      /** Thickness. */
      double m_Thickness;

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

      /** Makes objects storable. */
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
    const EndcapStructureGeometry* getEndcapStructureGeometry() const;

    /**
     * Get position data for endcaps.
     */
    const ElementPosition* getEndcapPosition() const;

    /**
     * Get position data for layers.
     */
    const ElementPosition* getLayerPosition() const;

    /**
     * Get Z distance between two layers.
     */
    double getLayerShiftZ() const;

    /**
     * Get position data for sectors.
     */
    const ElementPosition* getSectorPosition() const;

    /**
     * Get position data for sector support structure.
     */
    const ElementPosition* getSectorSupportPosition() const;

    /**
     * Get sector support geometry data.
     */
    const SectorSupportGeometry* getSectorSupportGeometry() const;

    /**
     * Get position data for planes.
     */
    const ElementPosition* getPlanePosition() const;

    /**
     * Get plastic sheet geometry data.
     */
    const PlasticSheetGeometry* getPlasticSheetGeometry() const;

    /**
     * Get segment support geometry data.
     */
    const SegmentSupportGeometry* getSegmentSupportGeometry() const;

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
    const StripGeometry* getStripGeometry() const;

    /**
     * Get position data for strips.
     * @param[in] strip Strip number.
     */
    const ElementPosition* getStripPosition(int strip) const;

    /**
     * Get shield layer details geometry data.
     */
    const ShieldGeometry* getShieldGeometry() const;

  protected:

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

    /** Solenoid center Z coordinate. */
    double m_SolenoidZ;

    /** Endcap structure geometry data. */
    EndcapStructureGeometry m_EndcapStructureGeometry;

    /** Position data for endcaps. */
    ElementPosition m_EndcapPosition;

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

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMGeometry, 2);

  };

}

#endif

