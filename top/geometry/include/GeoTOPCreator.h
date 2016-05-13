/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeometry.h>
#include <top/geometry/TOPGeometryPar.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

class G4LogicalVolume;
class G4AssemblyVolume;

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace TOP {

    class SensitivePMT;
    class SensitiveBar;

    /** Geometry creator for TOP counter.
     */
    class GeoTOPCreator : public geometry::CreatorBase {

    public:

      /** Constructor */
      GeoTOPCreator();

      /** Destructor */
      virtual ~GeoTOPCreator();

      /**
       * Creates the GEANT Objects for the TOP counter.
       * @param content A reference to the content part of the parameter description
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);

    private:

      /**
       * Create a parameter object from the Gearbox XML parameters.
       */
      const TOPGeometry* createConfiguration(const GearDir& param);

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


      SensitivePMT* m_sensitivePMT = 0;  /**< Sensitive vol. to register PMT hits */
      SensitiveBar* m_sensitiveBar = 0;  /**< Sensitive vol. to register particles */
      BkgSensitiveDetector* m_sensitivePCB1 = 0;  /**< PCB sensitive for BG studies */
      BkgSensitiveDetector* m_sensitivePCB2 = 0;  /**< PCB sensitive for BG studies */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< singleton class */
      const TOPGeometry* m_geo = 0; /**< Geometry parameters */
      int m_isBeamBkgStudy = 0; /**< flag for beam backgound simulation */
    };

  }
}

