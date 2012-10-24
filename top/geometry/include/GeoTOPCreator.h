/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOTOPCREATOR_H_
#define GEOTOPCREATOR_H_

#include <top/geometry/TOPGeometryPar.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <G4AssemblyVolume.hh>
#include <G4LogicalVolume.hh>

namespace Belle2 {
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

    protected:

      SensitivePMT* m_sensitivePMT;  /**< Sensitive vol. to register PMT hits */
      SensitiveBar* m_sensitiveBar;  /**< Sensitive vol. to register incoming particles */
      TOPGeometryPar* m_topgp;       /**< Geometry parameters from xml files */
      int isBeamBkgStudy;            /**< flag for beam backgound simulation */
    };

  }
}

#endif /* GEOTOPCREATOR_H_ */
