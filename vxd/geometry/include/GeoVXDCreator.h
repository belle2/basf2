/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter, Jozef Koval, Benjamin Schwenker           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOVXDCREATOR_H
#define GEOVXDCREATOR_H

#include <vxd/geometry/GeoVXDComponents.h>
#include <vxd/geometry/GeoVXDAssembly.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoVXDRadiationSensors.h>
#include <vxd/simulation/SensitiveDetectorBase.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <string>
#include <vector>
#include <map>


#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <vxd/dbobjects/VXDGeometryPar.h>


#include <G4Transform3D.hh>
#include <G4Polycone.hh>
class G4LogicalVolume;
class G4AssemblyVolume;
class G4VSolid;
class G4Material;
class G4UserLimits;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {

    /** The creator for the VXD geometry of the Belle II detector.   */
    class GeoVXDCreator : public geometry::CreatorBase {

    public:
      /** Constructor of the GeoVXDCreator class. */
      explicit GeoVXDCreator(const std::string& prefix);

      /** The destructor of the GeoVXDCreator class. */
      virtual ~GeoVXDCreator();

      /**
       * Read the sensor definitions from the database
       * @param sensors Reference to the database containing the parameters
       */
      virtual SensorInfoBase* createSensorInfo(const VXDGeoSensorPar& sensor) = 0;

      /**
       * Return a SensitiveDetector implementation for a given sensor
       * @param sensorID SensorID for the sensor
       * @param sensor   Information about the sensor to create the Sensitive Detector for
       */
      virtual SensitiveDetectorBase* createSensitiveDetector(VxdID sensorID, const VXDGeoSensor& sensor,
                                                             const VXDGeoSensorPlacement& placement) = 0;

      /**
       * Read parameters for given layer and store in m_ladder
       */
      virtual void setCurrentLayer(int layer, const VXDGeometryPar& parameters);

      /**
       * Place ladder corresponding to the given ladder id into volume
       * setLayer has to be called first to set the correct layer id
       */
      G4Transform3D placeLadder(int ladderID, double phi, G4LogicalVolume* volume, const G4Transform3D& placement,
                                const VXDGeometryPar& parameters);

      /**
       * Return the position where a daughter component is to be placed
       * @param mother Mother component
       * @param daugther Daughter component
       * @return Transformation matrix to place the daughter relative to the origin to the mother
       */
      G4Transform3D getPosition(const VXDGeoComponent& mother, const VXDGeoComponent& daughter, const VXDGeoPlacement& placement,
                                bool originCenter);

      /**
       * Get Alignment from paylead object
       * @param  params Payload object
       * @return Transformation matrix for component
       */
      G4Transform3D getAlignment(VXDAlignmentPar params);

      /**
       * Place a list of subcomponents into an component.
       *
       * If the volume of the given component is NULL, a new container will be
       * created to fit all subcomponents. It will have air as medium.  If
       * createContainer is true, then the placements above and below are
       * valid. If at least one subcomponent with this placement is found the
       * whole component is wrapped in a container volume with Air medium which
       * extends above and below to fit the subcomponents
       *
       * @param name Name for the potential new volume or as prefix for the
       *        container to extend the component
       * @param component Component to fit the subcomponents into @param
       *        placements Placement information for all subcomponents
       * @param createContainer if true, subcomponents are allowed to be placed
       *        on top or below the component and the whole component will be
       *        wrapped in an Air volume fitting all components
       * @return offset in w which was applied to the component when extending it
       */
      GeoVXDAssembly createSubComponents(const std::string& name, VXDGeoComponent&
                                         component, std::vector<VXDGeoPlacement> placements,
                                         bool originCenter = true, bool allowOutside = false);

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
      G4VSolid* createTrapezoidal(const std::string& name, double width, double
                                  width2, double length, double& height, double angle = 0);

      /** Create diamond radiation sensors. */
      void createDiamonds(const VXDGeoRadiationSensorsPar& params, G4LogicalVolume& topVolume, G4LogicalVolume& envelopeVolume);

      /**
       * Return vector of VXDGeoPlacements with all the components defined inside a given path
       */
      std::vector<VXDGeoPlacementPar> getSubComponents(GearDir path);

      /**
       * Read parameters for a ladder in layer with given ID from gearbox and layer
       * store them in payload
       */
      virtual void readLadder(int layer, GearDir components, VXDGeometryPar& geoparameters);

      /**
        * Read parameters for ladder components and their alignment corresponding
        * to the given ladder id
        */
      virtual void readLadderComponents(int layerID, int ladderID, GearDir content, VXDGeometryPar& vxdGeometryPar);

      /**
       * Read parameters for component name from Gearbox into geometry payload.
       * The name is assumed to be unique and Volumes are cached.
       * @param name Name of the component
       * @param components Path to components
       */
      void readComponent(const std::string& name, GearDir components, VXDGeometryPar& vxdGeometryPar);

      /**
       * Read parameters for all components in placement container from Gearbox
       * into payload.
       * @param placements container holding names of all components to be cached
       * @param componentDir Path to Gearbox where parameters are to be found
       */
      void readSubComponents(const std::vector<VXDGeoPlacementPar>& placements , GearDir componentsDir, VXDGeometryPar& vxdGeometryPar);

    protected:
      /** Prefix to prepend to all volume names */
      std::string m_prefix;
      /** GearDir pointing to the alignment parameters */
      GearDir m_alignment;
      /** GearDir pointing to the toplevel of the components */
      GearDir m_components;

      /** Cache of all previously created components */
      std::map<std::string, VXDGeoComponent> m_componentCache;
      /** Map containing Information about all defined sensor types */
      std::map<std::string, VXDGeoSensor> m_sensorMap;
      /** Parameters of the currently active ladder */
      VXDGeoLadder m_ladder;
      /** List to all created sensitive detector instances */
      std::vector<Simulation::SensitiveDetectorBase*> m_sensitive;
      /** Diamond radiation sensor "sub creator" */
      GeoVXDRadiationSensors m_radiationsensors;

      /** Name of the Material to be used for Air */
      std::string m_defaultMaterial;
      /** tolerance for Geant4 steps to be merged to a single step */
      float m_distanceTolerance {(float)(5 * Unit::um)};
      /** tolerance for the energy deposition in electrons to be merged in a single step */
      float m_electronTolerance {100};
      /** minimum number of electrons to be deposited by a particle to be saved */
      float m_minimumElectrons {10};
      /** Stepsize to be used inside active volumes */
      double m_activeStepSize {5 * Unit::um};
      /** Make also chips sensitive. */
      bool m_activeChips {false};
      /** Make sensitive detectors also see neutrons. */
      bool m_seeNeutrons {false};
      /** If true only create TrueHits from primary particles and ignore secondaries */
      bool m_onlyPrimaryTrueHits {false};
      /** If this is true, only active Materials will be placed for tracking
       * studies. Dead Material will be ignored */
      bool m_onlyActiveMaterial {false};
      /** Vector of G4UserLimit pointers */
      std::vector<G4UserLimits*> m_UserLimits;
      /// Current half-shell being processed (need to know ladder parent for hierarchy)
      std::string m_currentHalfShell {""};
      /// Used for translation of half-shell name into a VxdID to consitently handle it in hierarchy
      std::map<std::string, Belle2::VxdID> m_halfShellVxdIDs {
        {{"PXD.Ying"}, {Belle2::VxdID(1, 0, 0, 1)}},
        {{"PXD.Yang"}, {Belle2::VxdID(1, 0, 0, 2)}},
        {{"SVD.Pat"}, {Belle2::VxdID(3, 0, 0, 1)}},
        {{"SVD.Mat"}, {Belle2::VxdID(3, 0, 0, 2)}}
      };

    }; // class GeoVXDCreator

  } // namespace VXD
} //namespace Belle2

#endif /* GEOVXDCREATOR_H */
