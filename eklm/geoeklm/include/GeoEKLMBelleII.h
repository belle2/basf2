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

#include <string>
#include <vector>

namespace Belle2 {

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

    /*
     * create - creation of the detector geometry
     * @content: XML data directory
     * @topVolume: Geant world volume
     * @type:
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

    /*
     * createEndcap - create endcap
     * @iEndcap: number of endcap
     * @mlv: mother logical volume
     */
    void createEndcap(int iEndcap, G4LogicalVolume *mlv);

    /*
     * createLayer - create layer
     * @iLayer: number of layer
     * @mpvgt: mother physical volume with global transformation
     */
    void createLayer(int iLayer, G4PVPlacementGT *mpvgt);

    /*
     * createSector - create sector
     * @iSector: number of sector
     * @mpvgt: mother physical volume with global transformation
     */
    void createSector(int iSector, G4PVPlacementGT *mpvgt);

    /**
     * createSectorSupport - create sector support structure
     * @mlv: mother logical volume
     */
    void createSectorSupport(G4LogicalVolume *mlv);

    /**
     * createSectorSupportInnerTube - create inner tube of sector
     * support structure
     * @mlv: mother logical volume
     */
    G4Tubs *createSectorSupportInnerTube(G4LogicalVolume *mlv);

    /**
     * createSectorSupportOuterTube - create outer tube of sector
     * support structure
     * @mlv: mother logical volume
     */
    G4Tubs *createSectorSupportOuterTube(G4LogicalVolume *mlv);

    /**
     * createSectorSuportBoxX - create X side of sector support structure
     * @mlv: mother logical volume
     * @t: transformation (output)
     *
     * Sets t to the transformation of the box.
     */
    G4Box *createSectorSupportBoxX(G4LogicalVolume *mlv, G4Transform3D &t);

    /**
     * createSectorSuportBoxY - create Y side of sector support structure
     * @mlv: mother logical volume
     * @t: transformation (output)
     *
     * Sets t to the transformation of the box.
     */
    G4Box *createSectorSupportBoxY(G4LogicalVolume *mlv, G4Transform3D &t);

    /**
     * createSectorSuportBoxTop - create box in the cutted corner of sector
     * support structure
     * @mlv: mother logical volume
     * @t: transformation (output)
     *
     * Sets t to the transformation of the box.
     */
    G4Box *createSectorSupportBoxTop(G4LogicalVolume *mlv, G4Transform3D &t);

    /**
     * createPlane - create plane
     * @iPlane: number of plane
     * @mpvgt: mother physical volume with global transformation
     */
    void createPlane(int iPlane, G4PVPlacementGT *mpvgt);

    /**
     * createStrip - create strip
     * @iStrip: number of strip
     * @iPlane: number of plane
     * @mpvgt: mother physical volume with global transformation
     */
    void createStrip(int iStrip, int iPlane, G4PVPlacementGT *mpvgt);

    /**
     * Air
     */
    G4Material *Air;

    /**
     * Polystyrene
     */
    G4Material *Polystyrene;

    /**
     * Iron
     */
    G4Material *Iron;

    /**
     * Number of layers.
     */
    int nLayer;

    /**
     * Number of planes in one sector.
     */
    int nPlane;

    /**
     * Number of strips in one plane.
     */
    int nStrip;

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
     * Sector support structure thickness.
     */
    double SectorSupportThickness;

    /**
     * Sector support structure: Outer radius - Y coordinate of upper edge
     * of BoxY
     */
    double SectorSupport_DeltaLY;

    /**
     * Sector support structure: coordinate X of corner
     */
    double SectorSupport_CornerX;

    /**
     * Position data for planes.
     */
    struct EKLMElementPosition PlanePosition;

    /**
     * Position data for strips.
     */
    struct EKLMElementPosition *StripPosition;

    /**
     * Strip width.
     */
    double Strip_width;

    /**
     * Strip thickness.
     */
    double Strip_thickness;

    //! sensitive detector
    EKLMSensitiveDetector *m_sensitive;

  };


}

#endif /* GEOEKLMBelleII_H_ */
