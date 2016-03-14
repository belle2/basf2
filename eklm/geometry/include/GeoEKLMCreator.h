/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOEKLMBELLEII_H
#define GEOEKLMBELLEII_H

/* System headers. */
#include <sys/types.h>

/* C++ headers. */
#include <string>
#include <vector>

/* External headers. */
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4Transform3D.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4IntersectionSolid.hh>

/* Belle2 headers. */
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/G4TriangularPrism.h>
#include <eklm/geometry/GeoESTRCreator.h>
#include <eklm/geometry/GeoEKLMTypes.h>

/**
 * @file
 * EKLM geometry.
 */

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
     *
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
     * Readout board size data.
     */
    struct BoardSize {
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
     * Section support position.
     */
    struct SectionSupportPosition {
      double deltal_right;  /**< Right (X-plane) delta L. */
      double deltal_left;   /**< Left (X-plane) delta L. */
      double length;        /**< Length */
      double x;             /**< X coordinate. */
      double y;             /**< Y coordinate. */
      double z;             /**< Z coordinate. */
    };

    /**
     * Strip size data.
     */
    struct StripSize {
      double width;                      /**< Width. */
      double thickness;                  /**< Thickness. */
      double groove_depth;               /**< Groove depth. */
      double groove_width;               /**< Groove width. */
      double no_scintillation_thickness; /**< Non-scintillating layer. */
      double rss_size;                   /**< Radiation study SiPM size. */
    };

    /**
     * Sector support solids.
     */
    struct SectorSupportSolids {
      double c2x;      /**< Corner 2 X. */
      double c2y;      /**< Corner 2 Y. */
      double c3x;      /**< Corner 3 X. */
      double c3y;      /**< Corner 3 Y. */
      double c4x;      /**< Corner 4 X. */
      double c4y;      /**< Corner 4 Y. */
      G4VSolid* itube; /**< Inner tube. */
      G4VSolid* otube; /**< Outer tube. */
      G4VSolid* corn1; /**< Corner 1. */
      G4VSolid* corn2; /**< Corner 2. */
      G4VSolid* corn3; /**< Corner 3. */
      G4VSolid* corn4; /**< Corner 4. */
      G4VSolid* supp;  /**< Support. */
    };

    /**
     * Section support solids.
     */
    struct SectionSupportSolids {
      G4VSolid* topbox; /**< Top box. */
      G4VSolid* midbox; /**< Middle box. */
      G4VSolid* botbox; /**< Bottom box. */
      G4VSolid* us;     /**< Top box + middle box. */
      G4VSolid* secsup; /**< Section support. */
    };

    /**
     * Scintillator solids.
     */
    struct ScintillatorSolids {
      G4Box* box;               /**< Box (auxiliary). */
      G4SubtractionSolid* sens; /**< Sensitive area. */
    };

    /**
     * All solids of EKLM.
     */
    struct Solids {
      G4VSolid* endcap;                         /**< Endcap. */
      G4VSolid* layer;                          /**< Layer. */
      G4VSolid* sector;                         /**< Sector. */
      G4VSolid* cover;                          /**< Sector cover. */
      G4VSolid* secsupp;                        /**< Sector support. */
      G4VSolid** plane;                         /**< Plane. */
      G4VSolid** psheet;                        /**< Element of plastic sheet. */
      G4VSolid** stripvol;                      /**< Strip + SiPM volume. */
      G4VSolid** strip;                         /**< Strips. */
      G4VSolid** groove;                        /**< Strip grooves. */
      G4VSolid* sipm;                           /**< SiPM. */
      G4VSolid* board;                          /**< Readout board. */
      G4VSolid* baseboard;                      /**< Readout board base. */
      G4VSolid* stripboard;                     /**< Readout board for 1 strip. */
      struct SectorSupportSolids sectorsup;     /**< Sector support. */
      struct ScintillatorSolids* scint;         /**< Scintillator. */
      struct SectionSupportSolids** sectionsup; /**< Section support. */
    };

    /**
     * Logical volumes of EKLM.
     */
    struct LogicalVolumes {
      G4LogicalVolume** stripvol;    /**< Strip volumes. */
      G4LogicalVolume** strip;       /**< Strips. */
      G4LogicalVolume** groove;      /**< Strip grooves. */
      G4LogicalVolume** scint;       /**< Scintillator. */
      G4LogicalVolume** psheet;      /**< Plastic sheet. */
    };

    /**
     * Materials for EKLM.
     */
    struct Materials {
      G4Material* air;         /**< Air. */
      G4Material* polystyrene; /**< Polystyrene. */
      G4Material* polystyrol;  /**< Polystyrol. */
      G4Material* iron;        /**< Iron. */
      G4Material* duralumin;   /**< Duralumin. */
      G4Material* silicon;     /**< Silicon. */
      G4Material* gel;         /**< Gel. */
    };

    /**
     * Volume numbers.
     */
    struct VolumeNumbers {
      int endcap; /**< Endcap. */
      int layer;  /**< Layer. */
      int sector; /**< Sector. */
      int plane;  /**< Plane. */
      int strip;  /**< Strip. */
      int board;  /**< Board. */
    };

    /**
     * Class GeoEKLMCreator.
     * The creator for the  EKLM geometry of the Belle II detector.
     */
    class GeoEKLMCreator : public geometry::CreatorBase {

    public:

      /**
       * Default constructor.
       */
      GeoEKLMCreator();

      /**
       * Constructor with optional geometry data loading.
       * @param[in] geo True to load transfomation data.
       */
      GeoEKLMCreator(bool geo);

      /**
       * Destructor.
       */
      ~GeoEKLMCreator();

      /**
       * Creation of the detector geometry.
       * @param content   XML data directory.
       * @param topVolume Geant world volume.
       * @param type      Geometry type.
       */
      void create(const GearDir& content, G4LogicalVolume& topVolume,
                  geometry::GeometryTypes type);

      /**
       * Get endcap transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of endcap.
       * @details
       * Numbers start from 0.
       */
      void getEndcapTransform(HepGeom::Transform3D* t, int n);

      /**
       * Get layer transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of layer.
       * @details
       * Numbers start from 0.
       */
      void getLayerTransform(HepGeom::Transform3D* t, int n);

      /**
       * Get sector transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of sector.
       * @details
       * Numbers start from 0.
       */
      void getSectorTransform(HepGeom::Transform3D* t, int n);

      /**
       * Get plane transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of plane.
       * @details
       * Numbers start from 0.
       */
      void getPlaneTransform(HepGeom::Transform3D* t, int n);

      /**
       * Get strip transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of strip.
       * @details
       * Numbers start from 0.
       */
      void getStripTransform(HepGeom::Transform3D* t, int n);

    private:

      /**
       * Constructor actions.
       * @param[in] geo True to load transfomation data.
       */
      void constructor(bool geo);

      /**
       * Creation of materials.
       */
      void createMaterials();

      /**
       * Calculate sector support geometry data.
       */
      void calculateSectorSupportData();

      /**
       * Read strip parameters from XML database.
       */
      void readXMLDataStrips();

      /**
       * Read parameters from XML database.
       */
      void readXMLData();

      /**
       * Get plastic sheet element transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of list.
       * @details
       * Numbers start from 0.
       * This function is intended for construction of the list solids, so
       * the transformation does not include z shift.
       */
      void getSheetTransform(HepGeom::Transform3D* t, int n);

      /**
       * Create endcap solid.
       */
      void createEndcapSolid();

      /**
       * Create sector cover solid.
       */
      void createSectorCoverSolid();

      /**
       * Create sector support solid.
       */
      void createSectorSupportSolid();

      /**
       * Create sector support corner 1 solid.
       */
      void createSectorSupportCorner1Solid();

      /**
       * Create sector support corner 2 solid.
       */
      void createSectorSupportCorner2Solid();

      /**
       * Create sector support corner 3 solid.
       */
      void createSectorSupportCorner3Solid();

      /**
       * Create sector support corner 4 solid.
       */
      void createSectorSupportCorner4Solid();

      /**
       * Create plane solid.
       * @param[in] n Number of plane, from 0 to 1.
       */
      void createPlaneSolid(int n);

      /**
       * Create plastic sheet solids.
       * @param[in] n Number of sector, from 0 to 4.
       */
      void createPlasticSheetSolid(int n);

      /**
       * Create solids.
       */
      void createSolids();

      /**
       * Create endcap.
       * @param[in] mlv     Mother logical volume.
       */
      void createEndcap(G4LogicalVolume* mlv);

      /**
       * Create layer.
       * @param[in] mlv     Mother logical volume.
       */
      void createLayer(G4LogicalVolume* mlv);

      /**
       * Create sector.
       * @param[in] mlv     Mother logical volume.
       */
      void createSector(G4LogicalVolume* mlv);

      /**
       * Create sector cover.
       * @param[in] iCover Number of cover.
       * @param[in] mlv    Mother logical volume.
       */
      void createSectorCover(int iCover, G4LogicalVolume* mlv);

      /**
       * Create sector support structure.
       * @param[in] mlv   Mother logical volume.
       */
      void createSectorSupport(G4LogicalVolume* mlv);

      /**
       * Create inner tube of sector support structure.
       */
      G4Tubs* createSectorSupportInnerTube();

      /**
       * Create outer tube of sector support structure.
       */
      G4Tubs* createSectorSupportOuterTube();

      /**
       * Create X side of sector support structure.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxX(G4Transform3D& t);

      /**
       * Create Y side of sector support structure.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxY(G4Transform3D& t);

      /**
       * Create box in the cutted corner of sector support structure.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxTop(G4Transform3D& t);

      /**
       * Create sector support corner 1.
       * @param[in] mlv   Mother logical volume.
       */
      void createSectorSupportCorner1(G4LogicalVolume* mlv);

      /**
       * Create sector support corner 2.
       * @param[in] mlv   Mother logical volume.
       */
      void createSectorSupportCorner2(G4LogicalVolume* mlv);

      /**
       * Create sector support corner 3.
       * @param[in] mlv   Mother logical volume.
       */
      void createSectorSupportCorner3(G4LogicalVolume* mlv);

      /**
       * Create sector support corner 4.
       * @param[in] mlv   Mother logical volume.
       */
      void createSectorSupportCorner4(G4LogicalVolume* mlv);

      /**
       * Subtract board solids from planes.
       * @param[in] plane Plane solid without boards subtracted.
       * @param[in] n     Number of plane, from 0 to 1.
       */
      G4SubtractionSolid* subtractBoardSolids(G4SubtractionSolid* plane,
                                              int n);

      /**
       * Create plane.
       * @param[in] mlv    Mother logical volume.
       */
      void createPlane(G4LogicalVolume* mlv);

      /**
       * Create readout board.
       * @param[in] mlv    Mother logical volume.
       */
      void createSectionReadoutBoard(G4LogicalVolume* mlv);

      /**
       * Create base board of section readout board.
       * @param[in] mlv   Mother logical volume.
       */
      void createBaseBoard(G4LogicalVolume* mlv);

      /**
       * Create strip readout board.
       * @param[in] iBoard Number of board.
       * @param[in] mlv    Mother logical volume.
       */
      void createStripBoard(int iBoard, G4LogicalVolume* mlv);

      /**
       * Create section support.
       * @param[in] iSectionSupport Number of section support.
       * @param[in] mlv             Mother logical volume.
       */
      void createSectionSupport(int iSectionSupport, G4LogicalVolume* mlv);

      /**
       * Create plastic list element.
       * @param[in] iSheetPlane Number of list plane.
       * @param[in] iSheet      Number of list.
       * @param[in] mlv         Mother logical volume.
       */
      void createPlasticSheetElement(int iSheetPlane, int iSheet,
                                     G4LogicalVolume* mlv);

      /**
       * Create strip logical volumes.
       * @param[in] iStrip Number of strip in the length-based array.
       * @param[in] mlv    Mother logical volume.
       */
      void createStripLogicalVolumes(int iStrip, G4LogicalVolume* mlv);

      /**
       * Create strip volume.
       * @param[in] mlv    Mother logical volume.
       */
      void createStripVolume(G4LogicalVolume* mlv);

      /**
       * Create strip physical volumes.
       * @param[in] iStrip Number of strip in the length-based array.
       */
      void createStripPhysicalVolumes(int iStrip);

      /**
       * Create silicon cube in the place of SiPM for radiation study.
       * @param[in] mlv Mother logical volume.
       */
      void createSiPM(G4LogicalVolume* mlv);

      /**
       * Print mass of volume if m_mode == 2.
       * @param[in] lv  Logical volume.
       */
      void printVolumeMass(G4LogicalVolume* lv);

      /**
       * Allocate memory for volumes and set contents to zero where necessary.
       */
      void mallocVolumes();

      /**
       * Deallocate memory for volumes.
       */
      void freeVolumes();

      /**
       * Get cutted corner angle.
       * @return Angle.
       */
      double getSectorSupportCornerAngle();

      /**
       * Calculate board transformations.
       */
      void calcBoardTransform();

      /** Solids. */
      struct Solids solids;

      /** Logical volumes. */
      struct LogicalVolumes logvol;

      /** Materials. */
      struct Materials mat;

      /** Current volumes. */
      struct VolumeNumbers curvol;

      /** Geometry data. */
      GeometryData* m_geoDat;

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

      /** Number of strip readout boards on one section readout board. */
      int m_nStripBoard;

      /** Number of strips in one plane. */
      int m_nStrip;

      /** Number of sections is one plane. */
      int m_nSection;

      /** True if geometry is constructed. */
      bool haveGeo;

      /** Solenoid center Z coordinate. */
      double m_solenoidZ;

      /** ESTR Geometry data. */
      struct ESTRGeometryParams ESTRPar;

      /** Position data for endcaps. */
      struct ElementPosition EndcapPosition;

      /** Position data for layers. */
      struct ElementPosition LayerPosition;

      /** Z distance between two layers. */
      double Layer_shiftZ;

      /** Position data for sectors. */
      struct ElementPosition SectorPosition;

      /** Position data for sector support structure. */
      struct ElementPosition SectorSupportPosition;

      /** Sector support size data. */
      struct SectorSupportGeometry m_SectorSupportData;

      /** Readout board size data. */
      struct BoardSize BoardSize;

      /** Positions of readout boards. */
      struct BoardPosition* BoardPosition[2];

      /** Transformations of boards from sector reference frame. */
      G4Transform3D** BoardTransform[2];

      /** Positions of strip readout boards. */
      struct StripBoardPosition* StripBoardPosition;

      /** Position data for planes. */
      struct ElementPosition PlanePosition;

      /** Position data for section support structure. */
      struct SectionSupportPosition* SectionSupportPosition[2];

      /** Section support structure top box width. */
      double SectionSupportTopWidth;

      /** Section support structure top box thickness. */
      double SectionSupportTopThickness;

      /** Section support structure middle box width. */
      double SectionSupportMiddleWidth;

      /** Section support structure middle box thickness. */
      double SectionSupportMiddleThickness;

      /** Plastic list width. */
      double PlasticSheetWidth;

      /** Distance from edge of last strip to edge of plastic list. */
      double PlasticSheetDeltaL;

      /** Position data for strips. */
      struct ElementPosition* StripPosition;

      /** Strip size data. */
      struct StripSize StripSize;

      /** Detector mode. */
      enum EKLMDetectorMode m_mode;

      /** Sensitive detectors. */
      EKLMSensitiveDetector* m_sensitive[3];

    };

  }

}

#endif /* GEOEKLMBelleII_H_ */
