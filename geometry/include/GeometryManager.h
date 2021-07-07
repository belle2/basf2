/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <map>

class G4VPhysicalVolume;
class G4VisAttributes;

namespace Belle2 {

  class GearDir;
  class GeoConfiguration;
  class IntervalOfValidity;

  namespace geometry {
    class CreatorBase;

    /**
     * Flag indiciating the type of geometry to be used
     */
    enum GeometryTypes {
      FullGeometry,     /**< Full geometry for simulation */
      TrackingGeometry, /**< Simplified geometry for tracking purposes */
      DisplayGeometry   /**< Simplified geometry for display purposes */
    };

    /**
     * Class to manage the creation and conversion of the geometry
     *
     * This class is a singleton
     */
    class GeometryManager {
    public:
      /** Return a reference to the instance */
      static GeometryManager& getInstance();

      /**
       * Return a pointer to the top volume. Returns 0 if no geometry has been
       * created so far
       */
      G4VPhysicalVolume* getTopVolume() { return m_topVolume; }

      /**
       * Create Geometry.
       *
       * This function will create the geometry representation. First the
       * Materials will be created, than the top volume will be defined and
       * all components of the detector will be built by calling the
       * corresponding creator
       * @param params GearDir pointing to the top element of the geometry parameters
       * @param type Type of the geometry to be built
       */
      void createGeometry(const GearDir& params, GeometryTypes type = FullGeometry);

      /**
       * Create Geometry.
       *
       * This function will create the geometry representation from an existing
       * configuration object. First the Materials will be created, than the
       * top volume will be defined and all components of the detector will be
       * built by calling the corresponding creator
       * @param params geometry configuration object
       * @param type Type of the geometry to be built
       * @param useDB whether the creators should be using the Database or Gearbox
       */
      void createGeometry(const GeoConfiguration& params, GeometryTypes type = FullGeometry, bool useDB = true);

      /** Create Geometry configuration object.
       * This function will convert a Gearbox representation of the geometry to
       * an object containing all necessary parameters
       * @param detectorDir GearDir pointing to the top element of the geometry parameters
       * @param iov Interval of validity for the created object
       */
      GeoConfiguration createGeometryConfig(const GearDir& detectorDir, const IntervalOfValidity& iov);

      /**
       * Set the names of the components to create.
       * This member allows to set a list of component names. When creating the
       * the geometry, only components matching names in the list will be
       * created. All other components found in the parameters will be ignored
       * @param components List of detector components to be created
       */
      void setDetectorComponents(const std::vector<std::string>& components)
      {
        m_components.clear();
        m_components.insert(components.begin(), components.end());
      }

      /**
       * Set the names of the components to exclude from creation.
       * This member allows to set a list of component names. When creating the
       * the geometry, components matching names in the list will be ignored
       * @param components List of detector components to be excluded
       */
      void setExcludedComponents(const std::vector<std::string>& components)
      {
        m_excluded.clear();
        m_excluded.insert(components.begin(), components.end());
      }

      /**
       * Set the names of addtional components to be added to the default set.
       * This member allows to set a list of component names. When creating the
       * the geometry, components matching names in the list will be added if
       * they are not disabled by default. If a list of components is already
       * provided this list is ignored.
       *
       * @param components List of detector components to be added in addition
       */
      void setAdditionalComponents(const std::vector<std::string>& components)
      {
        m_additional.clear();
        m_additional.insert(components.begin(), components.end());
      }

      /** Set the density scaling: This is intended for systematic studies only
       * and will scale the density of all materials for each component in the
       * map by the given factor */
      void setDensityScaling(const std::map<std::string, double>& components)
      {
        m_densityScaling = components;
      }

      /** Choose whether a region should be assigned to each creator.
       * If set to true, all volumes created by each creator are assigned a
       * region with the name of the creator
       */
      void setAssignRegions(bool assignRegions)
      {
        m_assignRegions = assignRegions;
      }

      /**
       * Delete the existing Geant4 Geometry.
       */
      void clear();

      /**
       * Create a TGeo representation of the native geometry description.
       *
       * This method will convert the existing Geometry to a ROOT TGeo
       * representation which can be accessed using the global gGeoManager
       * provided by ROOT.
       */
      void createTGeoRepresentation();

      /**
       * Create an anonymous G4VisAttributes for an existing G4LogicalVolume
       *
       * @return Pointer to the new G4VisAttributes object
       */
      G4VisAttributes* newVisAttributes();

    private:
      /** Default constructor declared private since class is a Singleton. */
      GeometryManager(): m_topVolume(0) {};
      /** Copy constructor declared private since class is a Singleton. */
      GeometryManager(const GeometryManager&) = delete;
      /** Assignment operator declared private since class is a Singleton. */
      GeometryManager& operator=(const GeometryManager&) = delete;
      /** Destructor: make sure we clean up memory at the end */
      ~GeometryManager() {}

      /** Pointer to the top volume of the native geometry description */
      G4VPhysicalVolume* m_topVolume;
      /** List of all creators, to be freed when geometry is destucted */
      std::vector<CreatorBase*> m_creators;
      /** List of names of components to be created, all other components will be ignored */
      std::set<std::string> m_components;
      /** List of names of components to be excluded from creation */
      std::set<std::string> m_excluded;
      /** List of additional components to be added to the default set of components */
      std::set<std::string> m_additional;
      /** Scaling factors for densities */
      std::map<std::string, double> m_densityScaling;
      /** Bool to indicate whether we automatically assign a region with the
       * creator name to all volumes created by that creator
       */
      bool m_assignRegions {false};
      /** List of visualization attributes */
      std::vector<G4VisAttributes*> m_VisAttributes;
      /** Allow destruction of instance */
      friend struct std::default_delete<GeometryManager>;
    };

  }

} //Belle2 namespace
