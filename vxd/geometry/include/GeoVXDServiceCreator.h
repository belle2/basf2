/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOVXDSERVICECREATOR_H
#define GEOVXDSERVICECREATOR_H

#include <geometry/CreatorBase.h>

class G4Material;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {
    /** The creator for the VXD Service geometry of the Belle II detector. */
    class GeoVXDServiceCreator : public geometry::CreatorBase {
    public:
      /** Default constructor is sufficient in this case */
      GeoVXDServiceCreator() = default;

      /**
       * Creates the Geant4 Objects for the VXD Service geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the Geant4
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    private:
      /** Create the dock boxes */
      void createDockBoxes(const GearDir& content, G4LogicalVolume& topVolume);

      /** Default Material, inherited from topVolume */
      G4Material* m_defaultMaterial {0};
    };
  } // namespace VXD
} //namespace Belle2

#endif /* GEOVXDSERVICECREATOR_H */
