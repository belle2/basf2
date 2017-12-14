/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOECLCREATOR_H
#define GEOECLCREATOR_H

#include <vector>

#include <geometry/CreatorBase.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  class ECLCrystalsShapeAndPosition;
  namespace ECL {

    struct shape_t;



    //!  The GeoECLCreator class.
    /*!
       The creator for the ECL geometry of the Belle II detector.
    */
    class GeoECLCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoECLCreator class.
      GeoECLCreator();

      //! The destructor of the GeoECLCreator class.
      ~GeoECLCreator();

      /**
       * Function to actually create the geometry, has to be overridden by derived classes
       * @param content GearDir pointing to the parameters which should be used for construction
       * @param topVolume Top volume in which the geometry has to be placed
       * @param type Type of geometry to be build
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /**
       * Function to create the geometry from the Database
       * @param name name of the component in the database, could be used to disambiguate multiple components created with the same creator
       * @param topVolume Top volume in which the geometry has to be placed
       * @param type Type of geometry to be build
       */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /** Function to create the geometry database.
       * This function should be implemented to convert Gearbox parameters to one ore more database payloads
       * @param content GearDir pointing to the parameters which should be used for construction
       * @param iov interval of validity to use when generating payloads
       */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov);

    private:

      /** Make the ECL barrel and then place elements inside it */
      void barrel(G4LogicalVolume&);
      /** Place elements inside the backward endcap */
      void backward(G4LogicalVolume&);
      /** Place elements inside the forward endcap */
      void forward(G4LogicalVolume&);

      G4LogicalVolume* wrapped_crystal(const shape_t* s, const std::string& endcap, double wrapthickness);

      /** Define visual attributes */
      void defineVisAttributes();

      /** Define visual attributes
       * @param n Attribute name
       */
      const G4VisAttributes* att(const std::string& n) const;
      G4LogicalVolume* get_preamp() const ;
      double get_pa_box_height() const {return 2;}

      /** pointer to a storage with crystal shapes and positions */
      ECLCrystalsShapeAndPosition* m_sap;

      /** Sensitive detector */
      Simulation::SensitiveDetectorBase* m_sensitive;
      Simulation::SensitiveDetectorBase* m_sensediode;
      /** Vector of background-Sensitive detectors */
      std::map<std::string, G4VisAttributes*> m_atts;
      int m_overlap;
    };

  } // end of ecl namespace
} // end of Belle2 namespace

#endif /* GEOECLCREATOR_H */
