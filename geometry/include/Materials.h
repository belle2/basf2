/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MATERIALS_H
#define MATERIALS_H

#include <string>
#include <vector>
#include <framework/core/MRUCache.h>

class G4Material;
class G4Element;
class G4MaterialPropertiesTable;
class G4OpticalSurface;


namespace Belle2 {
  namespace gearbox { class Interface; }

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

      /**
       * Find given material. Shorthand for Materials::getInstance().getMaterial()
       * @see Materials::getMaterial
       * @param name of the material to find
       */
      static G4Material* get(const std::string &name) { return Materials::getInstance().getMaterial(name); }

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
       */
      G4Material* getMaterial(const std::string &name, bool showErrors = true) const;

      /**
       * Find given chemical element
       * @param name Name of the Element, e.g. Si, Al, He
       */
      G4Element* getElement(const std::string &name) const;

      /**
       * Create a material from the parameters specified by parameters
       * @param parameters Material properties
       * @return pointer to the newly created material, 0 if there was an
       *         error creating it
       */
      G4Material* createMaterial(const gearbox::Interface &parameters);

      /**
       * Create a list of Material properties from a set of parameters
       * @param  parameters Material properties definition
       * @return pointer to the newly created properties table, 0 if no
       *         properties were found
       */
      G4MaterialPropertiesTable* createProperties(const gearbox::Interface &parameter);

      /**
       * Create an optical surface from parameters
       * @param parameters Optical surface definition
       * @return new optical surface, 0 on error
       */
      G4OpticalSurface* createOpticalSurface(const gearbox::Interface &parameters);

    protected:
      /** Singleton: hide constructor */
      Materials(): m_materialCache(CacheSize) {};
      /** Singleton: hide copy constructor */
      Materials(const Materials&);
      /** Singleton: hide assignment operator */
      void operator=(const Materials&);

      /** Cache for already searched Materials */
      mutable MRUCache<std::string, G4Material*> m_materialCache;
    };

  } //geometry namespace

} //Belle2 namespace
#endif
