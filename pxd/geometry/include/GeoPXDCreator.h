/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOPXDCREATOR_H_
#define GEOPXDCREATOR_H_

#include <pxd/geometry/GeoPXDComponents.h>
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

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {

    class SensitiveDetector;

    /** The creator for the PXD geometry of the Belle II detector.   */
    class GeoPXDCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoPXDCreator class. */
      GeoPXDCreator();

      /** The destructor of the GeoPXDCreator class. */
      virtual ~GeoPXDCreator();

      /**
       * Creates the ROOT Objects for the PXD geometry.
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
      G4Transform3D getAlignment(const std::string& component);

      /**
       * Get the volume and the height representing a single sub-component
       * The name is assumed to be unique and Volumes are cached. If a component had allready
       * been created, the pointer to the existing volume will be returned
       * @param name Name of the component
       * @return pair containing the TGeoVolume* of the component as well as the height of the component
       */
      GeoPXDComponent getComponent(const std::string& component);

      /**
       * Place a list of subcomponents into an component.
       *
       * If the volume of the given component is NULL, a new container will be created to fit all subcomponents. It will have air as medium.
       * If createContainer is true, then the placements above and below are valid. If at least one subcomponent with this placement is found the whole
       * component is wrapped in a container volume with Air medium which extends above and below to fit the subcomponents
       * @param name Name for the potential new volume or as prefix for the container to extend the component
       * @param component Component to fit the subcomponents into
       * @param placements Placement information for all subcomponents
       * @param createContainer if true, subcomponents are allowed to be placed on top or below the component and the whole component will be wrapped in
       *            an Air volume fitting all components
       * @return offset in w which was applied to the component when extending it
       */
      double addSubComponents(const std::string& name, GeoPXDComponent &component, std::vector<GeoPXDPlacement> placements, bool createContainer = false, bool originCenter = true);

      /**
       * Return vector of GeoPXDPlacements with all the components defined inside a given path
       */
      std::vector<GeoPXDPlacement> getSubComponents(GearDir path);

      /**
       * Place ladder corresponding to the given ladder id into volume
       * setLayer has to be called first to set the correct layer id
       */
      void addLadder(int ladderID, double phi, G4LogicalVolume* volume, const G4Transform3D& placement);

      /**
       * Create support structure for PXD
       * @param support Reference to the database containing the parameters
       */
      G4AssemblyVolume* createSupport(GearDir support);

      /**
       * Create a PolyCone from parameters
       * @param name   Name of the volume
       * @param params Reference to the parameters defining the PolyCone
       */
      G4Polycone* createPolyCone(const std::string& name, GearDir params, double &minZ, double &maxZ);

      /**
       * Read parameters for given layer and store in m_ladder
       */
      void setLayer(int layer);



    protected:
      GearDir m_alignment;
      GearDir m_components;
      std::map<std::string, GeoPXDComponent> m_componentCache;
      GeoPXDLadder m_ladder;
      std::vector<SensitiveDetector*> m_sensitive;
    };

  }
}

#endif /* GEOPXDCREATOR_H_ */
