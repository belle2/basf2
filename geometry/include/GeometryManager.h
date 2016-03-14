/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include <vector>
#include <set>
#include <string>
#include <memory>

class G4VPhysicalVolume;

namespace Belle2 {

  class GearDir;

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
       * Materials will be created, than the top voluume will be defined and
       * all components of the detector will be built by calling the
       * corresponding creator
       * @param params GearDir pointing to the top element of the geometry parameters
       * @param type Type of the geometry to be built
       */
      void createGeometry(const GearDir& params, GeometryTypes type = FullGeometry);

      /**
       * Set the names of the components to create.
       * This member allows to set a list of component names. When creating the
       * the geometry, only components matching names in the list will be
       * created. All other components found in the parameters will be ignored
       * @param components List of detector components to be created
       */
      void setDetectorComponents(const std::vector<std::string>& components) {
        m_components.clear();
        m_components.insert(components.begin(), components.end());
      }

      /**
       * Set the names of the components to exclude from creation.
       * This member allows to set a list of component names. When creating the
       * the geometry, components matching names in the list will be ignored
       * @param components List of detector components to be excluded
       */
      void setExcludedComponents(const std::vector<std::string>& components) {
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
      void setAdditionalComponents(const std::vector<std::string>& components) {
        m_additional.clear();
        m_additional.insert(components.begin(), components.end());
      }

      void setAssignRegions(bool assignRegions) {
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

    private:
      /** Default constructor declared private since class is a Singleton. */
      GeometryManager(): m_topVolume(0) {};
      /** Destructor declared private since class is a Singleton */
      ~GeometryManager() { clear(); }
      /** Copy constructor declared private since class is a Singleton. */
      GeometryManager(const GeometryManager&);
      /** Assignment operator declared private since class is a Singleton. */
      void operator=(const GeometryManager&);

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
      /** Bool to indicate whether we automatically assign a region with the
       * creator name to all volumes created by that creator
       */
      bool m_assignRegions {false};

      /** Allow destruction of instance */
      friend class std::auto_ptr<GeometryManager>;
    };

  }

} //Belle2 namespace
#endif //GEOMETRYMANAGER_H
