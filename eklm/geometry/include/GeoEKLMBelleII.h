/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOEKLMBELLEII_H_
#define GEOEKLMBELLEII_H_

#include <sys/types.h>

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <eklm/geometry/TransformData.h>
#include <eklm/geometry/G4TriangularPrism.h>
#include <eklm/geometry/GeoESTRCreator.h>
#include <eklm/geometry/GeoEKLMTypes.h>

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4Transform3D.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4IntersectionSolid.hh>

#include <string>
#include <vector>

/**
 * @file
 * EKLM geometry.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Position information for the elements of detector.
     */
    struct ElementPosition {
      double innerR;   /**< Inner radius. */
      double outerR;   /**< Outer radius. */
      double length;   /**< Length. */
      double X;        /**< X coordinate. */
      double Y;        /**< Y coordinate. */
      double Z;        /**< Z coordinate. */
    };

    /**
     * Sector support size data.
     */
    struct SectorSupportSize {
      double Thickness;        /**< Yhickness. */
      double DeltaLY;          /**< outerR - Y of upper edge of BoxY. */
      double CornerX;          /**< Coordinate X of corner 1. */
      double TopCornerHeight;  /**< Corner 1 height (subtraction from plane). */
      double Corner1LX;        /**< Corner 1 X length. */
      double Corner1Width;     /**< Corner 1 width. */
      double Corner1Thickness; /**< Corner 1 thickness. */
      double Corner1Z;         /**< Corner 1 Z coordinate. */
      double Corner2LX;        /**< Corner 2 X length. */
      double Corner2LY;        /**< Corner 2 Y length. */
      double Corner2Thickness; /**< Corner 2 thickness. */
      double Corner2Z;         /**< Corner 2 Z coordinate. */
      double Corner3LX;        /**< Corner 3 X length. */
      double Corner3LY;        /**< Corner 3 Y length. */
      double Corner3Thickness; /**< Corner 3 thickness. */
      double Corner3Z;         /**< Corner 3 Z coordinate. */
      double Corner4LX;        /**< Corner 4 X length. */
      double Corner4LY;        /**< Corner 4 Y length. */
      double Corner4Thickness; /**< Corner 4 thickness. */
      double Corner4Z;         /**< Corner 4 Z coordinate. */
      double CornerAngle;      /**< Corner 1 angle. */
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
     * Section support solids.
     */
    struct SectionSupportSolids {
      G4Box* topbox;        /**< Top box. */
      G4Box* midbox;        /**< Middle box. */
      G4Box* botbox;        /**< Bottom box. */
      G4UnionSolid* us;     /**< Top box + middle box. */
      G4UnionSolid* secsup; /**< Section support. */
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
      G4VSolid* endcap;                     /**< Endcap. */
      G4VSolid** plane;                     /**< Plane. */
      G4VSolid** psheet;                    /**< Element of plastic sheet. */
      G4Box** stripvol;                     /**< Strip + SiPM volume. */
      G4Box** strip;                        /**< Strips. */
      G4Box** groove;                       /**< Strip grooves. */
      G4Box* sipm;                          /**< SiPM. */
      struct ScintillatorSolids* scint;     /**< Scintillator. */
      struct SectionSupportSolids** secsup; /**< Section support. */
    };

    /**
     * Logical volumes of EKLM.
     */
    struct LogicalVolumes {
      G4LogicalVolume** stripvol;    /**< Strip volumes. */
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
     * Class GeoEKLMBelleII.
     * The creator for the  EKLM geometry of the Belle II detector.
     */
    class GeoEKLMBelleII : public geometry::CreatorBase {

    public:

      /**
       * Default constructor.
       */
      GeoEKLMBelleII();

      /**
       * Constructor with optional geometry data loading.
       * @param[in] geo True to load transfomation data.
       */
      GeoEKLMBelleII(bool geo);

      /**
       * Destructor.
       */
      ~GeoEKLMBelleII();

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
       * @param[in] mlv   Mother logical volume.
       */
      G4Tubs* createSectorSupportInnerTube(G4LogicalVolume* mlv);

      /**
       * Create outer tube of sector support structure.
       * @param[in] mlv   Mother logical volume.
       */
      G4Tubs* createSectorSupportOuterTube(G4LogicalVolume* mlv);

      /**
       * Create X side of sector support structure.
       * @param[in]  mlv   Mother logical volume.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxX(G4LogicalVolume* mlv, G4Transform3D& t);

      /**
       * Create Y side of sector support structure.
       * @param[in]  mlv   Mother logical volume.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxY(G4LogicalVolume* mlv, G4Transform3D& t);

      /**
       * Create box in the cutted corner of sector support structure.
       * @param[in]  mlv   Mother logical volume.
       * @param[out] t     Transformation.
       * @details
       * Sets t to the transformation of the box.
       */
      G4Box* createSectorSupportBoxTop(G4LogicalVolume* mlv, G4Transform3D& t);

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
       * Create plastic list element
       * @param[in] iSheetPlane Number of list plane.
       * @param[in] iSheet      Number of list.
       * @param[in] mlv         Mother logical volume.
       */
      void createPlasticSheetElement(int iSheetPlane, int iSheet,
                                     G4LogicalVolume* mlv);

      /**
       * Create strip volume (strip + SiPM).
       * @param[in] mlv    Mother logical volume.
       */
      void createStripVolume(G4LogicalVolume* mlv);

      /**
       * Create strip.
       * @param[in] mlv Mother logical volume.
       */
      void createStrip(G4LogicalVolume* mlv);

      /**
       * Create strip groove.
       * @param[in] mlv Mother logical volume.
       */
      void createStripGroove(G4LogicalVolume* mlv);

      /**
       * Create strip sensitive volume.
       * @param[in] mlv Mother logical volume.
       */
      void createStripSensitive(G4LogicalVolume* mlv);

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

      /** Transformations. */
      struct TransformData* transf;

      /** Number of layers. */
      int nLayer;

      /** Number of planes in one sector. */
      int nPlane;

      /** Number of readout boards in one sector. */
      int nBoard;

      /** Number of strip readout boards on one section readout board. */
      int nStripBoard;

      /** Number of strips in one plane. */
      int nStrip;

      /** Number of sections is one plane. */
      int nSection;

      /** True if geometry data is loaded. */
      bool haveGeoDat;

      /** True if geometry is constructed. */
      bool haveGeo;

      /** ESTR Geometry data. */
      struct ESTR::GeometryParams ESTRPar;

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
      struct SectorSupportSize SectorSupportSize;

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
