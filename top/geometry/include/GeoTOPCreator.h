/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
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
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the TOP */
  namespace TOP {

    class SensitivePMT;
    class SensitiveTrack;

    /** The creator for the TOP geometry of the Belle II detector.   */
    class GeoTOPCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoTOPCreator class. */
      GeoTOPCreator();

      /** The destructor of the GeoTOPCreator class. */
      virtual ~GeoTOPCreator();

      /**
       * Creates the GEANT Objects for the TOP geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the GEANT
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /**
       * Creates a quartz bar
       * @param content A reference to the content part of the parameter description, which should to be used to create the GEANT objects.
       * @return G4AssemblyVolume of a whole bar
       */
      G4AssemblyVolume* buildBar(const GearDir& content);

      /**
       * Creates a segment of electronics which holds 8 PMTs
       * @param content A reference to the content part of the parameter description, which should to be used to create the GEANT objects.
       * @return G4AssemblyVolume of one segment of electronics
       */
      G4AssemblyVolume* buildElectronics(const GearDir& content);

      /**
       * Creates the support for the quartz bar, PMTs and electronics
       * @param content A reference to the content part of the parameter description, which should to be used to create the GEANT objects.
       * @return G4AssemblyVolume of a whole Support
       */
      G4LogicalVolume* buildSupport(const GearDir& content);

      /**
       * Creates the support for the quartz bar, PMTs and electronics
       * @param content A reference to the content part of the parameter description, which should to be used to create the GEANT objects.
       * @return G4LogicalVolume of the desired stack of PMTs
       */
      G4LogicalVolume* buildPMTstack(const GearDir& content);

      /**
       * Creates the support for the quartz bar, PMTs and electronics
       * @param content A reference to the content part of the parameter description, which should to be used to create the GEANT objects.
       * @return G4LogicalVolume of one PMT
       */
      G4LogicalVolume* buildPMT(const GearDir& content);

    protected:

      SensitivePMT* m_sensitivePMT;                  /**< Sensitive volume is used to store hits in the PMT (SimHits) */
      SensitiveTrack* m_sensitiveTrack;               /**< Sensitive volume is used to store hits of charges tracks at a small distance before the bar */
      TOPGeometryPar* m_topgp;                         /**< Used for reading parameters from the xml */
      int isBeamBkgStudy;                              /**< Used to store the flag is the beambackgound modules are ON or OFF */
    };

  }
}

#endif /* GEOTOPCREATOR_H_ */
