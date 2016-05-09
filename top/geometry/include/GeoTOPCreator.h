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
       * Creates a bar with a stack of PMT's
       * @param content A reference to the content part of the parameter description
       * @param moduleID the number used to read the proper alignement parameters.
       * @return G4AssemblyVolume of a whole bar
       */
      G4AssemblyVolume* buildBar(const GearDir& content, const int moduleID);

      /**
       * Creates a segment of electronics which holds 8 PMTs
       * @param content A reference to the content part of the parameter description
       * @return G4AssemblyVolume of one segment of electronics
       */
      G4AssemblyVolume* buildElectronics(const GearDir& content);

      /**
       * Creates one module of the TOP counter (bar+support+PMT array)
       * @param content A reference to the content part of the parameter description
       * @param moduleID the number used to read the proper alignement parameters.
       * @return G4LogicalVolume of a whole TOP module
       */
      G4LogicalVolume* buildTOPModule(const GearDir& content, const int moduleID);

      /**
       * Creates a PMT stack
       * @param content A reference to geometry description
       * @return G4AssemblyVolume of the desired stack of PMTs
       */
      G4LogicalVolume* buildPMTstack(const GearDir& content);

      /**
       * Creates a PMT
       * @param content A reference to geometry description
       * @return G4LogicalVolume of one PMT
       */
      G4LogicalVolume* buildPMT(const GearDir& content);


      SensitivePMT* m_sensitivePMT = 0;  /**< Sensitive vol. to register PMT hits */
      SensitiveBar* m_sensitiveBar = 0;  /**< Sensitive vol. to register particles */
      BkgSensitiveDetector* m_sensitivePCB1 = 0;  /**< PCB sensitive for BG studies */
      BkgSensitiveDetector* m_sensitivePCB2 = 0;  /**< PCB sensitive for BG studies */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< old singleton class */
      const TOPGeometry* m_geometry = 0; /**< Geometry parameters */
      int m_isBeamBkgStudy = 0; /**< flag for beam backgound simulation */
    };

  }
}

