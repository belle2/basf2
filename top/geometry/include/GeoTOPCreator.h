/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 * Major revision: 2016                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeometry.h>
#include <top/geometry/TOPGeometryPar.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>

class G4LogicalVolume;
class G4AssemblyVolume;
class G4Box;

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace TOP {

    class SensitivePMT;
    class SensitiveBar;

    /**
     * Geometry creator for TOP counter.
     */
    class GeoTOPCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor
       */
      GeoTOPCreator();

      /**
       * Destructor
       */
      virtual ~GeoTOPCreator();

      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type) override;

      /**
       * Creation of the detector geometry from database.
       * @param[in] name      Name of the component in the database.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume,
                                geometry::GeometryTypes type) override;

      /**
       * Creation of payloads.
       * @param content XML data directory.
       * @param iov     Interval of validity.
       */
      virtual void createPayloads(const GearDir& content,
                                  const IntervalOfValidity& iov) override;


    private:

      /**
       * Create the geometry from a parameter object.
       */
      void createGeometry(const TOPGeometry& parameters,
                          G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);

      /**
       * Creates one module of the TOP counter (bar+support+PMT array)
       * @param geo geometry description
       * @param moduleID module ID
       * @return logical volume of the module
       */
      G4LogicalVolume* buildTOPModule(const TOPGeometry& geo, int moduleID);

      /**
       * Creates a bar with a stack of PMT's
       * @param geo geometry description
       * @param moduleID module ID
       * @return assembly volume of the bar with PMT's
       */
      G4AssemblyVolume* buildBar(const TOPGeometry& geo, int moduleID);

      /**
       * Creates a PMT stack
       * @param geo geometry description
       * @return logical volume of PMT array
       */
      G4LogicalVolume* buildPMTstack(const TOPGeoPMTArray& geo);

      /**
       * Creates a PMT
       * @param geo geometry description
       * @return logical volume of one PMT
       */
      G4LogicalVolume* buildPMT(const TOPGeoPMT& geo);

      // -- new creator ----------------------------------------------------

      /**
       * Assembles optical components (PMT array, prism and bar segments) along z
       * @param geo geometry description
       * @param pmtArray pre-created PMT array logical volume
       * @param Lz array dimension in z
       * @return assembly volume
       */
      G4AssemblyVolume* assembleOptics(const TOPGeoModule& geo,
                                       G4LogicalVolume* pmtArray,
                                       double Lz);

      /**
       * Creates quartz bar segment
       * @param geo geometry description
       * @return logical volume of bar segment
       */
      G4LogicalVolume* createBarSegment(const TOPGeoBarSegment& geo);

      /**
       * Creates quartz prism
       * @param geo geometry description
       * @return logical volume of prism
       */
      G4LogicalVolume* createPrism(const TOPGeoPrism& geo);

      /**
       * Creates quartz segment with spherical mirror
       * @param geo geometry description
       * @return logical volume of mirror segment
       */
      G4LogicalVolume* createMirrorSegment(const TOPGeoMirrorSegment& geo);

      /**
       * Creates PMT array
       * @param geo geometry description
       * @return logical volume of PMT array
       */
      G4LogicalVolume* createPMTArray(const TOPGeoPMTArray& geo);

      /**
       * Creates single PMT
       * @param geo geometry description
       * @return logical volume of one PMT
       */
      G4LogicalVolume* createPMT(const TOPGeoPMT& geo);

      /**
       * Creates material box
       * @param name volume name
       * @param A box x size
       * @param B box y size
       * @param C box z size
       * @param materialName material name
       * @return logical volume
       */
      G4LogicalVolume* createBox(std::string name,
                                 double A, double B, double C,
                                 std::string materialName);

      /**
       * Creates material volume that is intersection of box and half-sphere shell (z > 0)
       * @param name volume name
       * @param box box shape
       * @param Rmin minimal radius of sphere shell
       * @param Rmax maximal radius of sphere shell
       * @param xc center of a sphere in x
       * @param yc center of a sphere in y
       * @param zc center of a sphere in z
       * @param materialName material name
       * @return logical volume
       */
      G4LogicalVolume* createBoxSphereIntersection(std::string name,
                                                   G4Box* box,
                                                   double Rmin, double Rmax,
                                                   double xc, double yc, double zc,
                                                   std::string materialName);

      /**
       * Adds number to string
       * @param str string
       * @param number number to be added
       * @return string with a number
       */
      std::string addNumber(const std::string& str, unsigned number);


      SensitivePMT* m_sensitivePMT = 0;  /**< Sensitive vol. to register PMT hits */
      SensitiveBar* m_sensitiveBar = 0;  /**< Sensitive vol. to register particles */
      BkgSensitiveDetector* m_sensitivePCB1 = 0;  /**< PCB sensitive for BG studies */
      BkgSensitiveDetector* m_sensitivePCB2 = 0;  /**< PCB sensitive for BG studies */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< singleton class */
      int m_isBeamBkgStudy = 0; /**< flag for beam backgound simulation */
    };

  }
}

