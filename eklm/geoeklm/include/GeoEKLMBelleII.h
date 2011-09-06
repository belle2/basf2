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

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>

#include <string>
#include <vector>

namespace Belle2 {

  static std::map<G4PVPlacement*, G4Transform3D> EKLMVolumeTransforms;

  //! Position information for the elements of detector
  struct EKLMElementPosition {
    double innerR;
    double outerR;
    double length;
    double X;
    double Y;
    double Z;
  };

  //!   The GeoEKLMBelleII class.
  //!   The creator for the outer EKLM geometry of the Belle II detector.

  class GeoEKLMBelleII : public geometry::CreatorBase {

  public:

    //! Constructor of the GeoEKLMBelleII class.
    GeoEKLMBelleII();

    //! The destructor of the GeoEKLMBelleII class.
    ~GeoEKLMBelleII();

    //! Create geometry
    virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                        geometry::GeometryTypes type);

    //! returns top volume for EKLM
//    inline TGeoVolume * getTopmostVolume()
//    {return volGrpEKLM;}
  protected:

  private:

    //! Create materials
    void createMaterials();

    //! Read position data
    void readPositionData(struct EKLMElementPosition& epos,
                          GearDir& content);

    //! Read XML data
    void readXMLData(const GearDir& content);

    /* Create endcap */
    void createEndcap(int iEndcap, G4LogicalVolume *mlv);

    /* Create layer */
    void createLayer(int iLayer, G4LogicalVolume *mlv, G4Transform3D *mtr,
                     int zOrient);

    /* Create sector */
    void createSector(int iSector, G4LogicalVolume *mlv, G4Transform3D *mtr,
                      int zOrient);

    /* Create plane */
    void createPlane(int iPlane, int iSector, G4LogicalVolume *mlv,
                     G4Transform3D *mtr, int zOrient);

    /* Create strip */
    void createStrip(int iStrip, int iPlane, int iSector, G4LogicalVolume *mlv,
                     G4Transform3D *mtr);

    //! Materials
    G4Material *Air, *Polystyrene, *Iron;

    //! Numbers of detector elements
    int nEndcap, nLayer, nSector, nPlane, nStrip;

    //! Detector parameters
    double EKLM_OffsetZ, Layer_shiftZ, Strip_width, Strip_thickness;

    //! Positions of elements
    struct EKLMElementPosition EndcapPosition, LayerPosition, SectorPosition,
          PlanePosition, *StripPosition;

    //! Names
    std::string Endcap_Name, Layer_Name, Sector_Name, Plane_Name, Strip_Name;

    //! sensitive detector
    EKLMSensitiveDetector *m_sensitive;

  };


}

#endif /* GEOEKLMBelleII_H_ */
