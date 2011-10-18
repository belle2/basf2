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
#include <eklm/simeklm/EKLMBkgSensitiveDetector.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4Transform3D.hh>
#include <G4SubtractionSolid.hh>

#include <string>
#include <vector>

namespace Belle2 {

  class G4TriangularPrism;

  /**
   * struct EKLMElementPosition - position information for the elements
   * of detector
   */
  struct EKLMElementPosition {
    double innerR;   /**< inner radius */
    double outerR;   /**< outer radius */
    double length;   /**< length */
    double X;        /**< X coordinate */
    double Y;        /**< Y coordinate */
    double Z;        /**< Z coordinate */
  };

  /**
   * struct EKLMSectorSupportSize - sector support size data
   */
  struct EKLMSectorSupportSize {
    double Thickness;        /**< thickness */
    double DeltaLY;          /**< outerR - Y of upper edge of BoxY */
    double CornerX;          /**< coordinate X of corner 1 */
    double TopCornerHeight;  /**< corner 1 height (subtraction from plane) */
    double Corner1LX;        /**< corner 1 X length */
    double Corner1Width;     /**< corner 1 width */
    double Corner1Thickness; /**< corner 1 thickness */
    double Corner1Z;         /**< corner 1 Z coordinate */
    double Corner2LX;        /**< corner 2 X length */
    double Corner2LY;        /**< corner 2 Y length */
    double Corner2Thickness; /**< corner 2 thickness */
    double Corner2Z;         /**< corner 2 Z coordinate */
    double Corner3LX;        /**< corner 3 X length */
    double Corner3LY;        /**< corner 3 Y length */
    double Corner3Thickness; /**< corner 3 thickness */
    double Corner3Z;         /**< corner 3 Z coordinate */
    double Corner4LX;        /**< corner 4 X length */
    double Corner4LY;        /**< corner 4 Y length */
    double Corner4Thickness; /**< corner 4 thickness */
    double Corner4Z;         /**< corner 4 Z coordinate */
    double CornerAngle;      /**< corner 1 angle */
  };

  /**
   * struct EKLMBoardSize - readout board size data
   */
  struct EKLMBoardSize {
    double length;       /**< length */
    double width;        /**< width */
    double height;       /**< height */
    double base_width;   /**< width of base board */
    double base_height;  /**< height of base board */
    double strip_length; /**< length of strip readout board */
    double strip_width;  /**< width of strip readout board */
    double strip_height; /**< height of strip readout board */
  };

  /**
   * struct EKLMStripBoardPosition - strip readout board position data
   */
  struct EKLMStripBoardPosition {
    double x;         /**< x coordinate */
  };

  /**
   * struct EKLMBoardPosition - readout board position data
   */
  struct EKLMBoardPosition {
    double r;      /**< radius of far edge of the board */
    double phi;    /**< angle */
  };

  /**
   * struct EKLMSectionSupportPosition - section support position
   */
  struct EKLMSectionSupportPosition {
    double deltal_right;  /**< right (X-plane) delta L */
    double deltal_left;   /**< left (X-plane) delta L */
    double length;        /**< length */
    double x;             /**< X coordinate */
    double y;             /**< Y coordinate */
    double z;             /**< Z coordinate */
  };

  /**
   * Struct EKLMStripSize - strip size data.
   */
  struct EKLMStripSize {
    double width;                      /**< width */
    double thickness;                  /**< thickness */
    double groove_depth;               /**< groove depth */
    double groove_width;               /**< groove width */
    double no_scintillation_thickness; /**< non-scintillating layer */
    double rss_size;                   /**< radiation study SiPM size */
  };

  /**
   * Class GeoEKLMBelleII.
   * The creator for the  EKLM geometry of the Belle II detector.
   */
  class GeoEKLMBelleII : public geometry::CreatorBase {

  public:

    /**
     * Constructor of the GeoEKLMBelleII class.
     */
    GeoEKLMBelleII();

    /**
     * Destructor of the GeoEKLMBelleII class.
     */
    ~GeoEKLMBelleII();

    /**
     * create - creation of the detector geometry
     * @content: XML data directory
     * @topVolume: Geant world volume
     * @type: geometry type
     */
    virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                        geometry::GeometryTypes type);

  protected:

  private:

    /**
     * Create materials
     */
    void createMaterials();

    /**
     * Read position data
     */
    void readPositionData(struct EKLMElementPosition& epos,
                          GearDir& content);

    /**
     * Read XML data
     */
    void readXMLData(const GearDir& content);

    /**
     * ReflectCoordinates - get reflected coordinates
     */
    void ReflectCoordinates(double xin, double yin, double *xout, double *yout,
                            int quarter);

    /**
     * createEndcap - create endcap
     * @iEndcap: number of endcap
     * @mlv: mother logical volume
     */
    void createEndcap(int iEndcap, G4LogicalVolume *mlv);

    /**
     * createLayer - create layer
     * @iLayer: number of layer
     * @iEndcap: number of endcap
     * @mpvgt: mother physical volume with global transformation
     */
    void createLayer(int iLayer, int iEndcap, G4PVPlacementGT *mpvgt);

    /**
     * createSector - create sector
     * @iSector: number of sector
     * @mpvgt: mother physical volume with global transformation
     */
    void createSector(int iSector, G4PVPlacementGT *mpvgt);

    /**
     * createSectorCover - create sector cover
     * @iCover: number of cover
     * @@mpvgt: mother physical volume with global transformation
     */
    void createSectorCover(int iCover, G4PVPlacementGT *mpvgt);

    /**
     * createSectorSupport - create sector support structure
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectorSupport(G4PVPlacementGT *mpvgt);

    /**
     * createSectorSupportInnerTube - create inner tube of sector
     * support structure
     * @mpvgt: mother physical volume with global transformation
     */
    G4Tubs *createSectorSupportInnerTube(G4PVPlacementGT *mpvgt);

    /**
     * createSectorSupportOuterTube - create outer tube of sector
     * support structure
     * @mpvgt: mother physical volume with global transformation
     */
    G4Tubs *createSectorSupportOuterTube(G4PVPlacementGT *mpvgt);

    /**
     * createSectorSuportBoxX - create X side of sector support structure
     * @mpvgt: mother physical volume with global transformation
     * @t: transformation (output)
     *
     * Sets t to the transformation of the box.
     */
    G4Box *createSectorSupportBoxX(G4PVPlacementGT *mpvgt, G4Transform3D &t);

    /**
     * createSectorSuportBoxY - create Y side of sector support structure
     * @mpvgt: mother physical volume with global transformation
     * @t: transformation (output)
     *
     * Sets t to the transformation of the box.
     */
    G4Box *createSectorSupportBoxY(G4PVPlacementGT *mpvgt, G4Transform3D &t);

    /**
     * createSectorSuportBoxTop - create box in the cutted corner of sector
     * support structure
     * @mpvgt: mother physical volume with global transformation
     * @t: transformation (output)
     *
     * Sets t to the transformation of the box.
     */
    G4Box *createSectorSupportBoxTop(G4PVPlacementGT *mpvgt, G4Transform3D &t);

    /**
     * createSectorSupportCorner1 - create sector support corner 1
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectorSupportCorner1(G4PVPlacementGT *mpvgt);

    /*
     * createSectorSupportCorner2 - create sector support corner 2
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectorSupportCorner2(G4PVPlacementGT *mpvgt);

    /*
     * createSectorSupportCorner3 - create sector support corner 3
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectorSupportCorner3(G4PVPlacementGT *mpvgt);

    /*
     * createSectorSupportCorner4 - create sector support corner 4
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectorSupportCorner4(G4PVPlacementGT *mpvgt);

    /**
     * subtractBoardSolids - subtract board solids from planes
     * @plane: plane solid without boards subtracted
     * @iPlane: plane number
     * @Plane_name: plane name
     */
    G4SubtractionSolid *subtractBoardSolids(G4SubtractionSolid *plane,
                                            int iPlane, std::string Plane_Name);

    /**
     * createPlane - create plane
     * @iPlane: number of plane
     * @mpvgt: mother physical volume with global transformation
     */
    void createPlane(int iPlane, G4PVPlacementGT *mpvgt);

    /**
     * createSectionReadoutBoard - create readout board
     * @iPlane: number of plane
     * @iBoard: number of board
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectionReadoutBoard(int iPlane, int iBoard,
                                   G4PVPlacementGT *mpvgt);

    /**
     * createBaseBoard - create base board of section readout board
     * @mpvgt: mother physical volume with global transformation
     */
    void createBaseBoard(G4PVPlacementGT *mpvgt);

    /**
     * createStripBoard - create strip readout board
     * @iBoard: number of board
     * @mpvgt: mother physical volume with global transformation
     */
    void createStripBoard(int iBoard, G4PVPlacementGT *mpvgt);

    /**
     * createSectionSupport - create section support
     * @iSectionSupport: number of section support
     * @iPlane: number of plane
     * @mpvgt: mother physical volume with global transformation
     */
    void createSectionSupport(int iSectionSupport, int iPlane,
                              G4PVPlacementGT *mpvgt);

    /**
     * createPlasticListElement - create plastic list element
     * @iListPlane: number of list plane
     * @iList: number of list
     * @mpvgt: mother physical volume with global transformation
     */
    void createPlasticListElement(int iListPlane, int iList,
                                  G4PVPlacementGT *mpvgt);

    /**
     * createStripVolume - create strip volume (strip + SiPM)
     * @iStrip: number of strip
     * @mpvgt: mother physical volume with global transformation
     */
    void createStripVolume(int iStrip, G4PVPlacementGT *mpvgt);

    /**
     * createStrip - create strip
     * @iStrip: number of strip
     * @mpvgt: mother physical volume with global transformation
     */
    void createStrip(int iStrip, G4PVPlacementGT *mpvgt);

    /**
     * createStripGroove - create strip groove
     * @iStrip: number of strip
     * @mpvgt: mother physical volume with global transformation
     */
    void createStripGroove(int iStrip, G4PVPlacementGT *mpvgt);

    /**
     * createStripSensitive - create strip sensitive volume
     * @iStrip: number of strip
     * @mpvgt: mother physical volume with global transformation
     */
    void createStripSensitive(int iStrip, G4PVPlacementGT *mpvgt);

    /**
     * createSiPM - create silicon cube in the place of SiPM for radiation study
     * @iStrip: number of strip
     * @mpvgt: mother physical volume with global transformation
     */
    void createSiPM(int iStrip, G4PVPlacementGT *mpvgt);

    /**
     * Air.
     */
    G4Material *Air;

    /**
     * Polystyrene.
     */
    G4Material *Polystyrene;

    /**
     * Polystyrol.
     */
    G4Material *Polystyrol;

    /**
     * Iron.
     */
    G4Material *Iron;

    /**
     * Duralumin.
     */
    G4Material *Duralumin;

    /**
     * Silicon.
     */
    G4Material *Silicon;

    /**
     * Gel.
     */
    G4Material *Gel;

    /**
     * Number of layers.
     */
    int nLayer;

    /**
     * Number of planes in one sector.
     */
    int nPlane;

    /**
     * Number of readout boards in one sector.
     */
    int nBoard;

    /**
     * Number of strip readout boards on one section readout board.
     */
    int nStripBoard;

    /**
     * Number of strips in one plane.
     */
    int nStrip;

    /**
     * Number of sections is one plane
     */
    int nSection;

    /**
     * Position data for endcaps.
     */
    struct EKLMElementPosition EndcapPosition;

    /**
     * Position data for layers.
     */
    struct EKLMElementPosition LayerPosition;

    /**
     * Z distance between two layers.
     */
    double Layer_shiftZ;

    /**
     * Position data for sectors.
     */
    struct EKLMElementPosition SectorPosition;

    /**
     * Position data for sector support structure.
     */
    struct EKLMElementPosition SectorSupportPosition;

    /**
     * Sector support size data.
     */
    struct EKLMSectorSupportSize SectorSupportSize;

    /**
     * getSectorSupportCornerAngle - get cutted corner angle
     */
    double getSectorSupportCornerAngle();

    /**
     * Readout board size data
     */
    struct EKLMBoardSize BoardSize;

    /**
     * Positions of readout boards
     */
    struct EKLMBoardPosition *BoardPosition[2];

    /**
     * Transformations of boards from sector reference frame
     */
    G4Transform3D *BoardTransform[2];

    /**
     * Calculate board transformations
     */
    void calcBoardTransform();

    /**
     * Positions of strip readout boards.
     */
    struct EKLMStripBoardPosition *StripBoardPosition;

    /**
     * Position data for planes.
     */
    struct EKLMElementPosition PlanePosition;

    /**
     * Position data for section support structure.
     */
    struct EKLMSectionSupportPosition *SectionSupportPosition[2];

    /**
     * Section support structure top box width.
     */
    double SectionSupportTopWidth;

    /**
     * Section support structure top box thickness.
     */
    double SectionSupportTopThickness;

    /**
     * Section support structure middle box width.
     */
    double SectionSupportMiddleWidth;

    /**
     * Section support structure middle box thickness.
     */
    double SectionSupportMiddleThickness;

    /**
     * Plastic list width
     */
    double PlasticListWidth;

    /**
     * Distance from edge of last strip to edge of plastic list
     */
    double PlasticListDeltaL;

    /**
     * Position data for strips.
     */
    struct EKLMElementPosition *StripPosition;

    /**
     * Strip size data.
     */
    struct EKLMStripSize StripSize;

    /**
     * Detector mode. See description of possible values in G4PVPlacementGT.h
     */
    int m_mode;

    /**
     * Sensitive detector
     */
    EKLMSensitiveDetector *m_sensitive;

    /**
     * Background study sensitive detectors.
     * 0 - strips,
     * 1 - SiPMs,
     * 2 - readout boards.
     */
    EKLMBkgSensitiveDetector *m_bkgsensitive[3];

  };


}

#endif /* GEOEKLMBelleII_H_ */
