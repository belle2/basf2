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
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <G4Transform3D.hh>
class G4LogicalVolume;
class G4AssemblyVolume;
class G4Polycone;
class G4Material;
namespace Belle2 {
  namespace top {

    class SensitiveDetector;
    class SensitiveQuartz;

    /** The creator for the TOP geometry of the Belle II detector.   */
    class GeoTOPCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoPXDCreator class. */
      GeoTOPCreator();

      /** The destructor of the GeoPXDCreator class. */
      virtual ~GeoTOPCreator();

      /**
       * Creates the ROOT Objects for the TOP geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /**
       * Get Alignment for given component from the database
       * @param  component Name of the component to align
       * @return Transformation matrix for component, idendity if component
       *         could not be found
       */

      //! This function quartz bar
      G4AssemblyVolume* buildBar(const GearDir& content);
      //! This function constructs the case for the quertz bar
      G4LogicalVolume* buildSupport(const GearDir& content);
      //! This function constructs a stack of 2x16 PMTS
      G4LogicalVolume* buildPMTstack(const GearDir& content);
      //! This function constructs one pmt
      G4LogicalVolume* buildPMT(const GearDir& content);

    protected:
      //! This sensitive volume is used to store hits in the PMT
      SensitiveDetector* m_sensitive;
      //! This sensitive volume is used to store hits in the qurtz and is used as loong the trackign is not finished
      SensitiveQuartz* m_sensitiveQuartz;
      //! used for reading parameters from the xml
      TOPGeometryPar* m_topgp;
      int isBeamBkgStudy;
    };

  }
}

#endif /* GEOTOPCREATOR_H_ */
