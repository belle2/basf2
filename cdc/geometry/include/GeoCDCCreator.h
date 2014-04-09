/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOCDCCREATOR_H
#define GEOCDCCREATOR_H

#include <geometry/CreatorBase.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cdc/simulation/CDCSensitiveDetector.h>

class G4LogicalVolume;
class G4VPhysicalVolume;

namespace Belle2 {
  namespace CDC {

    //!  The GeoCDCCreator class.
    /*!
       The creator for the CDC geometry of the Belle II detector.
    */
    class GeoCDCCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoCDCCreator class.
      GeoCDCCreator();

      //! The destructor of the GeoCDCCreator class.
      ~GeoCDCCreator();

      //! Creates the ROOT Objects for the CDC geometry.
      /*!
        \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);


    protected:

    private:

      //! CDC G4 logical volume.
      G4LogicalVolume* logical_cdc;

      //! CDC G4 physical volume.
      G4VPhysicalVolume* physical_cdc;

      //! Sensitive detector
      CDCSensitiveDetector* m_sensitive;


    };

  } // end of cdc namespace
} // end of Belle2 namespace

#endif /* GEOCDCBELLEII_H */
