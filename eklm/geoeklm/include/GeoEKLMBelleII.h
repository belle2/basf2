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

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <eklm/simeklm/EKLMSensitiveDetector.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>

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

namespace Belle2 {

  class G4TriangularPrism;

  /**
   * Position information for the elements of detector.
   */
  struct EKLMElementPosition {
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
  struct EKLMSectorSupportSize {
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
  struct EKLMBoardSize {
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
  struct EKLMStripBoardPosition {
    double x;         /**< X coordinate. */
  };

  /**
   * Readout board position data.
   */
  struct EKLMBoardPosition {
    double r;      /**< Radius of far edge of the board. */
    double phi;    /**< Angle. */
  };

  /**
   * Section support position.
   */
  struct EKLMSectionSupportPosition {
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
  struct EKLMStripSize {
    double width;                      /**< Width. */
    double thickness;                  /**< Thickness. */
    double groove_depth;               /**< Groove depth. */
    double groove_width;               /**< Groove width. */
    double no_scintillation_thickness; /**< Non-scintillating layer. */
    double rss_size;                   /**< Radiation study SiPM size. */
  };

  /**
   * Plane solids.
   */
  struct EKLMPlaneSolids {
    G4Tubs* tube;                   /**< Tube. */
    G4Box* box1;                    /**< Box. */
    G4Box* box2;                    /**< Box to subtract corner 1. */
    G4TriangularPrism* prism1;      /**< Corner 2. */
    G4TriangularPrism* prism2;      /**< Corner 3. */
    G4TriangularPrism* prism3;      /**< Corner 4. */
    G4IntersectionSolid* is;        /**< Arc. */
    G4SubtractionSolid* ss1;        /**< Arc - corner 1. */
    G4SubtractionSolid* ss2;        /**< Arc - corners 1, 2. */
    G4SubtractionSolid* ss3;        /**< Arc - corners 1, 2, 3. */
    G4SubtractionSolid* ss4;        /**< Arc - all corners. */
    G4SubtractionSolid* plane;      /**< Arc - all corners - boards. */
  };

  /**
   * Section support solids.
   */
  struct EKLMSectionSupportSolids {
    G4Box* topbox;        /**< Top box. */
    G4Box* midbox;        /**< Middle box. */
    G4Box* botbox;        /**< Bottom box. */
    G4UnionSolid* us;     /**< Top box + middle box. */
    G4UnionSolid* secsup; /**< Section support. */
  };

  /**
   * Scintillator solids.
   */
  struct EKLMScintillatorSolids {
    G4Box* box;               /**< Box (auxiliary). */
    G4SubtractionSolid* sens; /**< Sensitive area. */
  };

  /**
   * All solids for EKLM.
   */
  struct EKLMSolids {
    G4Box** list;                             /**< Element of plastic list. */
    G4Box** stripvol;                         /**< Strip + SiPM volume. */
    G4Box** strip;                            /**< Strips. */
    G4Box** groove;                           /**< Strip grooves. */
    struct EKLMScintillatorSolids* scint;     /**< Scintillator. */
    struct EKLMPlaneSolids* plane;            /**< Plane. */
    struct EKLMSectionSupportSolids** secsup; /**< Section support. */
  };

  /**
   * Materials for EKLM.
   */
  struct EKLMMaterials {
    G4Material* air;         /**< Air. */
    G4Material* polystyrene; /**< Polystyrene. */
    G4Material* polystyrol;  /**< Polystyrol. */
    G4Material* iron;        /**< Iron. */
    G4Material* duralumin;   /**< Duralumin. */
    G4Material* silicon;     /**< Silicon. */
    G4Material* gel;         /**< Gel. */
  };

  /**
   * Class GeoEKLMBelleII.
   * The creator for the  EKLM geometry of the Belle II detector.
   */
  class GeoEKLMBelleII : public geometry::CreatorBase {

  public:

    /**
     * Constructor.
     */
    GeoEKLMBelleII();

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

  protected:

  private:

    /**
     * Creation of materials.
     */
    void createMaterials();

    /**
     * Read parameters from XML database
     * @param[in] content GearDir to read from.
     */
    void readXMLData(const GearDir& content);

    /**
     * Create endcap
     * @param[in] iEndcap Number of endcap.
     * @param[in] mlv     Mother logical volume.
     */
    void createEndcap(int iEndcap, G4LogicalVolume* mlv);

    /**
     * Create layer.
     * @param[in] iLayer  Number of layer.
     * @param[in] iEndcap Number of endcap.
     * @param[in] mpvgt   Mother physical volume with global transformation.
     */
    void createLayer(int iLayer, int iEndcap, G4PVPlacementGT* mpvgt);

    /**
     * Create sector.
     * @param[in] iSector Number of sector.
     * @param[in] mpvgt   Mother physical volume with global transformation.
     */
    void createSector(int iSector, G4PVPlacementGT* mpvgt);

    /**
     * Create sector cover
     * @param[in] iCover Number of cover.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createSectorCover(int iCover, G4PVPlacementGT* mpvgt);

    /**
     * Create sector support structure.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    void createSectorSupport(G4PVPlacementGT* mpvgt);

    /**
     * Create inner tube of sector support structure.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    G4Tubs* createSectorSupportInnerTube(G4PVPlacementGT* mpvgt);

    /**
     * Create outer tube of sector support structure.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    G4Tubs* createSectorSupportOuterTube(G4PVPlacementGT* mpvgt);

    /**
     * Create X side of sector support structure.
     * @param[in]  mpvgt Mother physical volume with global transformation.
     * @param[out] t     Transformation.
     * @details
     * Sets t to the transformation of the box.
     */
    G4Box* createSectorSupportBoxX(G4PVPlacementGT* mpvgt, G4Transform3D& t);

    /**
     * Create Y side of sector support structure.
     * @param[in]  mpvgt Mother physical volume with global transformation.
     * @param[out] t     Transformation.
     * @details
     * Sets t to the transformation of the box.
     */
    G4Box* createSectorSupportBoxY(G4PVPlacementGT* mpvgt, G4Transform3D& t);

    /**
     * Create box in the cutted corner of sector support structure.
     * @param[in]  mpvgt Mother physical volume with global transformation.
     * @param[out] t     Transformation.
     * @details
     * Sets t to the transformation of the box.
     */
    G4Box* createSectorSupportBoxTop(G4PVPlacementGT* mpvgt, G4Transform3D& t);

    /**
     * Create sector support corner 1.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    void createSectorSupportCorner1(G4PVPlacementGT* mpvgt);

    /**
     * Create sector support corner 2.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    void createSectorSupportCorner2(G4PVPlacementGT* mpvgt);

    /**
     * Create sector support corner 3.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    void createSectorSupportCorner3(G4PVPlacementGT* mpvgt);

    /**
     * Create sector support corner 4.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    void createSectorSupportCorner4(G4PVPlacementGT* mpvgt);

    /**
     * Subtract board solids from planes.
     * @param[in] plane      Plane solid without boards subtracted.
     * @param[in] iPlane     Plane number.
     * @param[in] Plane_name Plane name.
     */
    G4SubtractionSolid* subtractBoardSolids(G4SubtractionSolid* plane,
                                            int iPlane, std::string Plane_Name);

    /**
     * Create plane.
     * @param[in] iPlane Number of plane.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createPlane(int iPlane, G4PVPlacementGT* mpvgt);

    /**
     * Create readout board.
     * @param[in] iPlane Number of plane.
     * @param[in] iBoard Number of board.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createSectionReadoutBoard(int iPlane, int iBoard,
                                   G4PVPlacementGT* mpvgt);

    /**
     * Create base board of section readout board.
     * @param[in] mpvgt Mother physical volume with global transformation.
     */
    void createBaseBoard(G4PVPlacementGT* mpvgt);

    /**
     * Create strip readout board.
     * @param[in] iBoard Number of board.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createStripBoard(int iBoard, G4PVPlacementGT* mpvgt);

    /**
     * Create section support.
     * @param[in] iSectionSupport Number of section support.
     * @param[in] iPlane          Number of plane.
     * @param[in] mpvgt           Mother physical volume
     *                            with global transformation.
     */
    void createSectionSupport(int iSectionSupport, int iPlane,
                              G4PVPlacementGT* mpvgt);

    /**
     * Create plastic list element
     * @param[in] iListPlane Number of list plane.
     * @param[in] iList      Number of list.
     * @param[in] mpvgt      Mother physical volume with global transformation.
     */
    void createPlasticListElement(int iListPlane, int iList,
                                  G4PVPlacementGT* mpvgt);

    /**
     * Create strip volume (strip + SiPM).
     * @param[in] iStrip Number of strip.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createStripVolume(int iStrip, G4PVPlacementGT* mpvgt);

    /**
     * Create strip.
     * @param[in] iStrip Number of strip.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createStrip(int iStrip, G4PVPlacementGT* mpvgt);

    /**
     * Create strip groove.
     * @param[in] iStrip Number of strip.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createStripGroove(int iStrip, G4PVPlacementGT* mpvgt);

    /**
     * Create strip sensitive volume.
     * @param[in] iStrip Number of strip.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createStripSensitive(int iStrip, G4PVPlacementGT* mpvgt);

    /**
     * Create silicon cube in the place of SiPM for radiation study.
     * @param[in] iStrip Number of strip.
     * @param[in] mpvgt  Mother physical volume with global transformation.
     */
    void createSiPM(int iStrip, G4PVPlacementGT* mpvgt);

    /**
     * Print mass of volume if m_mode == 2.
     * @param[in] lv  Logical volume.
     */
    void printVolumeMass(G4LogicalVolume* lv);

    /**
     * Allocate memory for solids and set contents to zero.
     */
    void mallocSolids();

    /**
     * Deallocate memory for solids.
     */
    void freeSolids();

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
    struct EKLMSolids solids;

    /** Materials. */
    struct EKLMMaterials mat;

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

    /** Position data for endcaps. */
    struct EKLMElementPosition EndcapPosition;

    /** Position data for layers. */
    struct EKLMElementPosition LayerPosition;

    /** Z distance between two layers. */
    double Layer_shiftZ;

    /** Position data for sectors. */
    struct EKLMElementPosition SectorPosition;

    /** Position data for sector support structure. */
    struct EKLMElementPosition SectorSupportPosition;

    /** Sector support size data. */
    struct EKLMSectorSupportSize SectorSupportSize;

    /** Readout board size data. */
    struct EKLMBoardSize BoardSize;

    /** Positions of readout boards. */
    struct EKLMBoardPosition* BoardPosition[2];

    /** Transformations of boards from sector reference frame. */
    G4Transform3D** BoardTransform[2];

    /** Positions of strip readout boards. */
    struct EKLMStripBoardPosition* StripBoardPosition;

    /** Position data for planes. */
    struct EKLMElementPosition PlanePosition;

    /** Position data for section support structure. */
    struct EKLMSectionSupportPosition* SectionSupportPosition[2];

    /** Section support structure top box width. */
    double SectionSupportTopWidth;

    /** Section support structure top box thickness. */
    double SectionSupportTopThickness;

    /** Section support structure middle box width. */
    double SectionSupportMiddleWidth;

    /** Section support structure middle box thickness. */
    double SectionSupportMiddleThickness;

    /** Plastic list width. */
    double PlasticListWidth;

    /** Distance from edge of last strip to edge of plastic list. */
    double PlasticListDeltaL;

    /** Position data for strips. */
    struct EKLMElementPosition* StripPosition;

    /** Strip size data. */
    struct EKLMStripSize StripSize;

    /** Detector mode. */
    enum EKLMDetectorMode m_mode;

    /** Sensitive detector. */
    EKLMSensitiveDetector* m_sensitive;

    /** File to store transformation matrices. */
    std::string m_outputFile;

  };

}

#endif /* GEOEKLMBelleII_H_ */
