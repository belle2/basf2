/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOTOPTBCREATOR_H_
#define GEOTOPTBCREATOR_H_

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <G4AssemblyVolume.hh>
#include <G4LogicalVolume.hh>


namespace Belle2 {
  namespace TOPTB {

    class SensitiveScintillator;

    /**
     * Geometry creator for TOP test beam telescope counters etc.
     */
    class GeoTOPTBCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor
       */
      GeoTOPTBCreator();

      /**
       * Destructor
       */
      virtual ~GeoTOPTBCreator();

      /**
       * Creates GEANT4 objects
       * @param content a content part of DetectorComponent
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);


    protected:

      /**
       * creates a test beam setup
       * @param content setup parameters
       * @param elements an elements part of
       */
      G4AssemblyVolume* createSetup(const GearDir& content, const GearDir& elements);

      /**
       * creates an element of a given type for the test beam setup
       * @param elements an elements part of
       * @param elementType type of the element
       * @param elementName name of the element
       * @param detectorID detector ID
       */
      G4LogicalVolume* createElement(const GearDir& elements,
                                     std::string elementType,
                                     std::string elementName,
                                     int detectorID);

      /**
       * returns transformation for setup placement
       * @param content placement parameters
       * @param methodName transformation method (e.g. the meaning of parameters)
       */
      G4Transform3D getTransformation(const GearDir& content,
                                      std::string methodName = "");

      /**
       * returns rotation matrix
       * @param content rotation parameters
       */
      G4RotationMatrix* getRotation(const GearDir& content);

      /**
       * returns translation vector
       * @param content translation parameters
       */
      G4ThreeVector getTranslation(const GearDir& content);

      /**
       * creates scintillator counter
       * @param content counter paramaters
       * @param elementName element name in the setup
       * @param detectorID detector ID
       */
      G4LogicalVolume* scintillatorCounter(const GearDir& content,
                                           std::string elementName,
                                           int detectorID);

      /**
       * creates scintillating fiber tracker
       * @param content tracker parameters
       * @param elementName element name in the setup
       * @param detectorID detector ID
       */
      G4LogicalVolume* sciFiTracker(const GearDir& content,
                                    std::string elementName,
                                    int detectorID);


      /**
       * creates veto counter
       * @param content counter parameters
       * @param elementName element name in the setup
       * @param detectorID detector ID
       */
      G4LogicalVolume* vetoCounter(const GearDir& content,
                                   std::string elementName,
                                   int detectorID);




      /**
       * list of sensitive scintillators (needed to be known to class destructor)
       */
      std::vector<SensitiveScintillator*> m_sensitiveScintillators;



    };

  } // namespace TOPTB
} // namespace Belle2

#endif
