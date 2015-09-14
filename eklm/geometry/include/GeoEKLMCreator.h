/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_GEOEKLMCREATOR_H
#define EKLM_GEOEKLMCREATOR_H

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
#include <eklm/geometry/GeometryData2.h>
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
     * Segment support solids.
     */
    struct SegmentSupportSolids {
      G4VSolid* topbox; /**< Top box. */
      G4VSolid* midbox; /**< Middle box. */
      G4VSolid* botbox; /**< Bottom box. */
      G4VSolid* us;     /**< Top box + middle box. */
      G4VSolid* secsup; /**< Segment support. */
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
      struct SegmentSupportSolids** segmentsup; /**< Segment support. */
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
       * Read strip parameters from XML database.
       */
      void readXMLDataStrips();

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
       * Cut corner of a solid.
       * @param[in] name           Name of resulting solid.
       * @param[in] solid          Solid.
       * @param[in] subtractionBox Box used for subtractions.
       * @param[in] transf         Additional transformation of subtraction box.
       * @param[in] largerAngles   If true then cut larger angles.
       * @param[in] x1             First point x coordinate.
       * @param[in] y1             First point y coordinate.
       * @param[in] x2             Second point x coordinate.
       * @param[in] y2             Second point y coordinate.
       */
      G4SubtractionSolid* cutSolidCorner(
        char* name, G4VSolid* solid, G4Box* subtractionBox,
        HepGeom::Transform3D& transf, bool largerAngles,
        double x1, double y1, double x2, double y2);

      /**
       * Cut corner of a solid.
       * @param[in] name           Name of resulting solid.
       * @param[in] solid          Solid.
       * @param[in] subtractionBox Box used for subtractions.
       * @param[in] transf         Additional transformation of subtraction box.
       * @param[in] largerAngles   If true then cut larger angles.
       * @param[in] x              Initial point x coordinate.
       * @param[in] y              Initial point y coordinate.
       * @param[in] ang            Angle.
       */
      G4SubtractionSolid* cutSolidCorner(
        char* name, G4VSolid* solid, G4Box* subtractionBox,
        HepGeom::Transform3D& transf, bool largerAngles,
        double x, double y, double ang);

      /**
       * Subtract board solids from planes.
       * @param[in] plane Plane solid without boards subtracted.
       * @param[in] n     Number of plane, from 0 to 1.
       */
      G4SubtractionSolid* subtractBoardSolids(G4SubtractionSolid* plane,
                                              int n);

      /**
       * Create plane solid.
       * @param[in] n Number of plane, from 0 to 1.
       */
      void createPlaneSolid(int n);

      /**
       * Unify a group of solids.
       * @param[in] solids  Solids.
       * @param[in] transf  Transformations.
       * @param[in] nSolids Number of solids.
       * @param[in] name    First part of solid names.
       */
      G4UnionSolid* unifySolids(G4VSolid** solids, HepGeom::Transform3D* transf,
                                int nSolids, std::string name);

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
       * Create plane.
       * @param[in] mlv    Mother logical volume.
       */
      void createPlane(G4LogicalVolume* mlv);

      /**
       * Create readout board.
       * @param[in] mlv    Mother logical volume.
       */
      void createSegmentReadoutBoard(G4LogicalVolume* mlv);

      /**
       * Create base board of segment readout board.
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
       * Create segment support.
       * @param[in] iSegmentSupport Number of segment support.
       * @param[in] mlv             Mother logical volume.
       */
      void createSegmentSupport(int iSegmentSupport, G4LogicalVolume* mlv);

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

      /** Geometry data. */
      const GeometryData2* m_geoDat2;

      /** True if geometry is constructed. */
      bool haveGeo;

      /** ESTR Geometry data. */
      struct ESTRGeometryParams ESTRPar;

      /** Transformations of boards from sector reference frame. */
      G4Transform3D** BoardTransform[2];

      /** Sensitive detectors. */
      EKLMSensitiveDetector* m_sensitive[3];

    };

  }

}

#endif /* GEOEKLMBelleII_H_ */
