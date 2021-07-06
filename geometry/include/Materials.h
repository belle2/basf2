/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/MRUCache.h>

#include <optional>
#include <set>
#include <string>
#include <vector>

class G4Material;
class G4Element;
class G4MaterialPropertiesTable;
class G4OpticalSurface;
class G4NistElementBuilder;
class G4NistMaterialBuilder;

namespace Belle2 {
  namespace gearbox { class Interface; }

  class GeoMaterial;
  class GeoOpticalSurface;
  class GeoMaterialProperty;

  namespace geometry {
    /**
     * Thin wrapper around the Geant4 Material system
     *
     * This class serves the purpose to simplify the access to the Geant4
     * materials using the G4NistManager and to directly create materials from a
     * Gearbox parameter set. Since Geant4 keeps track of all defined materials,
     * this class is also able to find Materials created directly by the user.
     *
     * Care should be taken when a Material is created with a name that already
     * exists. In that case it may be possible that we return the wrong material
     * since the first material with the given name will be returned.
     */
    class Materials {
    public:
      enum { CacheSize = 100 };

      /** Get a reference to the singleton instance */
      static Materials& getInstance();

      /** Destructor for objects that I have created */
      ~Materials();

      /**
       * Find given material. Shorthand for Materials::getInstance().getMaterial()
       * @see Materials::getMaterial
       * @param name of the material to find
       */
      static G4Material* get(const std::string& name) { return Materials::getInstance().getMaterial(name); }

      /**
       * Get a pointer to the G4Material with the given name
       *
       * If the given name cannot be found we also try to find it in the Nist
       * database by prepending the name with G4_ and trying in with given case
       * and all uppercase since single element materials are defined as e.g.
       * G4_Si, G4_Al and other materials are defined all uppercase, e.g.
       * G4_AIR, G4_TEFLON.
       *
       * Found materials are cached internaly using an MRU cache so repitive
       * access to materials should be O(1) in most cases
       *
       * @param name Name of the material to be found
       * @param showErrors If false don't show any errors, just return nullptr if the material cannot be found
       */
      G4Material* getMaterial(const std::string& name, bool showErrors = true);

      /**
       * Find given chemical element
       * @param name Name of the Element, e.g. Si, Al, He
       */
      G4Element* getElement(const std::string& name);

      /**
       * Create a material from the parameters specified by parameters
       * @param parameters Material properties
       * @return pointer to the newly created material, 0 if there was an
       *         error creating it
       */
      G4Material* createMaterial(const gearbox::Interface& parameters);

      /** Create Geant4 Material from Material definition in DB
       * @param parameters Material properties
       * @return pointer to the newly created material, 0 if there was an
       *         error creating it
       */
      G4Material* createMaterial(const GeoMaterial& parameters);

      /**
       * Create an optical surface from parameters, will abort on error
       * @param parameters Optical surface definition
       * @return new optical surface
       */
      G4OpticalSurface* createOpticalSurface(const gearbox::Interface& parameters);

      /**
       * Create an optical surface from Surface definition in DB
       * @param surface Optical surface definition
       * @return new optical surface
       */
      G4OpticalSurface* createOpticalSurface(const GeoOpticalSurface& surface);

      /**
       * Create Material from XML description
       * @param parameters GearDir pointing to the Material description
       * @return GeoMaterial instance describing the material and suitable for
       * serialization
       */
      GeoMaterial createMaterialConfig(const gearbox::Interface& parameters);

      /** Create Optical Surface Configuration from XML description
       * @param parameters GearDir pointing to the Optical Surface description
       * @return GeoOpticalSurface instance describing the material and
       * suitable for serialization
       */
      GeoOpticalSurface createOpticalSurfaceConfig(const gearbox::Interface& parameters);

      /** Clear all existing materials */
      void clear();

      /** Set the density scale to the given value */
      void setDensityScale(double scale)
      {
        if (scale == 1.0) return resetDensityScale();
        m_densityScale = scale;
      }
      /** Disable density scaling */
      void resetDensityScale()
      {
        m_densityScale.reset();
      }
      /** get the set of material names which will **not** be scaled in any way */
      std::set<std::string> getDensityScaleIgnoredMaterials() const { return m_ignoreScaling; }
      /** set the set of material names which will **not** be scaled in any way */
      void setDensityScaleIgnoredMaterials(const std::set<std::string>& ignored) { m_ignoreScaling = ignored; }
    protected:
      /** Singleton: hide constructor */
      Materials(): m_materialCache(CacheSize) {};
      /** Singleton: hide copy constructor */
      Materials(const Materials&) = delete;
      /** Singleton: hide assignment operator */
      void operator=(const Materials&) = delete;
      /** find an existing material by name */
      G4Material* findMaterial(const std::string& name);

      /** Cache for already searched Materials */
      mutable MRUCache<std::string, G4Material*> m_materialCache;

      /** Vector of created G4MaterialProperties objects */
      std::vector<G4MaterialPropertiesTable*> m_PropTables;

      /** G4NistElementBuilder instance to create chemical elements with
       * correct natural abundances from NIST database */
      G4NistElementBuilder* m_nistElementBuilder{nullptr};
      /** G4NistMaterialBuilder to create predefined materials from NIST database */
      G4NistMaterialBuilder* m_nistMaterialBuilder{nullptr};

      /** Create a properties table from a vector of properties. Return 0 if the vector is empty */
      G4MaterialPropertiesTable* createProperties(const std::vector<GeoMaterialProperty>& props);

      /** Initialize Nist Builder instances */
      void initBuilders();

      /** If set we scale all densities by a given factor */
      std::optional<double> m_densityScale;
      /** Names of materials we don't want to scale */
      std::set<std::string> m_ignoreScaling{"G4_AIR", "Air", "Vacuum"};
    };
  } //geometry namespace
} //Belle2 namespace
