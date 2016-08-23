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

class G4LogicalVolume;
class shape_t;

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace ECL {

    class SensitiveDetector;

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

      //! Creates the ROOT Objects for the ECL geometry.
      /*!
        \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /** Make the ECL barrel and then place elements inside it */
      void barrel(const GearDir&, G4LogicalVolume&);
      /** Place elements inside the backward endcap */
      void backward(const GearDir&, G4LogicalVolume&);
      /** Place elements inside the forward endcap */
      void forward(const GearDir&, G4LogicalVolume&);

    protected:
      /** isBeamBkgStudy for neutron flux  */
      int isBeamBkgStudy;

    private:

      G4LogicalVolume* wrapped_crystal(const shape_t* s, const std::string& endcap, double wrapthickness);

      /** Sensitive detector */
      SensitiveDetector* m_sensitive;
      /** Vector of background-Sensitive detectors */
      std::vector<BkgSensitiveDetector*> m_bkgsensitive;

    };

  } // end of ecl namespace
} // end of Belle2 namespace

#endif /* GEOECLCREATOR_H */
