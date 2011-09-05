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
      void createGeometry(const GearDir &params, GeometryTypes type = FullGeometry);

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

      /** Allow destruction of instance */
      friend class std::auto_ptr<GeometryManager>;
    };

  }

} //Belle2 namespace
#endif //GEOMETRYMANAGER_H
