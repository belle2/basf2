/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Jozef Koval                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSVDCREATOR_H
#define GEOSVDCREATOR_H

#include <svd/geometry/GeoSVDComponents.h>
#include <svd/simulation/SensitiveDetector.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <G4Transform3D.hh>

#include <string>
#include <vector>
#include <map>

class G4LogicalVolume;
class G4Polycone;

namespace Belle2 {

  namespace svd {
    /** The creator for the SVD geometry of the Belle II detector.   */
    class GeoSVDCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoSVDCreator class. */
      GeoSVDCreator();

      /** The destructor of the GeoSVDCreator class. */
      virtual ~GeoSVDCreator();

      /**
       * Creates the ROOT Objects for the PXD geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /** Create a trapezoidal solid.
       * @param width full forward width of the shape in mm
       * @param width2 full backward width of the shape in mm
       * @param length length of the shape in mm
       * @param[inout] height of the shape in mm. If angle is not 0 this value
       *        might be changed if the actual height will be smaller due to
       *        the slanted edges
       * @param angle angle of the sides along w with respect to to the uv
       *        plane. 0 means normal box shaped, !=0 means the upper endcap of
       *        the solid will be smaller since all edges will be slanted by
       *        angle
       * @return A G4VShape which could be a G4Box, a G4Trd or a G4Trap depending on the parameters
       */
      G4VSolid* createTrapezoidal(const std::string& name, double width, double width2, double length, double &height, double angle = 0);

      /**
       * Get the volume and the height representing a single sub-component
       * The name is assumed to be unique and Volumes are cached. If a component had already
       * been created, the pointer to the existing volume will be returned
       * @param name Name of the component
       * @return pair containing the TGeoVolume* of the component as well as the height of the component
       */
      GeoSVDComponent getComponent(const std::string& component);

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
      double addSubComponents(const std::string& name, GeoSVDComponent &component, std::vector<GeoSVDPlacement> placements, bool createContainer = false, bool originCenter = true);

      /**
       * Return vector of GeoPXDPlacements with all the components defined inside a given path
       */
      std::vector<GeoSVDPlacement> getSubComponents(GearDir path);

      /**
       * Get Alignment for given component from the database
       * @param  component Name of the component to align
       * @return Transformation matrix for component, identity if component
       *         could not be found
       */
      G4Transform3D getAlignment(const std::string& component);

      /**
       * Place ladder corresponding to the given ladder id into volume
       * setLayer has to be called first to set the correct layer id
       */
      void addLadder(int ladderID, double phi, G4LogicalVolume* volume, const G4Transform3D& placement);

      /**
       * Read parameters for given layer and store in m_ladder
       */
      void setLayer(int layer);

    protected:
      GearDir m_alignment;
      GearDir m_components;
      std::map<std::string, GeoSVDComponent> m_componentCache;
      std::map<int, GeoSVDSensor> m_sensorMap;
      std::map<int, GeoSVDSensor>::iterator m_sensorMapIterator;
      GeoSVDLadder m_ladder;
      std::vector<SensitiveDetector*> m_sensitive;


    }; // class GeoSVDCreator
  }  // namespece svd
}  // namespace Belle2

#endif /* GEOSVDCREATOR_H */
