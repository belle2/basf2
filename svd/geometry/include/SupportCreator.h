/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SUPPORTCREATOR_H_
#define SUPPORTCREATOR_H_

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <string>
#include <vector>

#include <G4Transform3D.hh>
class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {

    /** The creator for the SVD Support geometry of the Belle II detector.   */
    class SupportCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the SupportCreator class. */
      SupportCreator();

      /** The destructor of the SupportCreator class. */
      virtual ~SupportCreator();

      /**
       * Creates the Objects for the PXD geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    };

  }
}

#endif /* GEOPXDCREATOR_H_ */
