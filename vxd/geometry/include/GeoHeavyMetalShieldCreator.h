/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jacek Stypula                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOHEAVYMETALSHIELDCREATOR_H
#define GEOHEAVYMETALSHIELDCREATOR_H

#include <geometry/CreatorBase.h>

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {

    /** The creator for the HeavyMetalShield geometry of the Belle II detector.   */
    class GeoHeavyMetalShieldCreator : public geometry::CreatorBase {

    public:
      /**
       * Creates the ROOT Objects for the HeavyMetalShield geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    };
  }
}

#endif /* GEOHEAVYMETALSHIELDCREATOR_H */
