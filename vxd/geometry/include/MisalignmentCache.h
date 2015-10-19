/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_MISALIGNMENT_CACHE_H
#define VXD_MISALIGNMENT_CACHE_H

#include <vxd/dataobjects/VxdID.h>
#include <unordered_map>

#include <TGeoMatrix.h>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {
    /**
     * Class to hold misalignment information
     */
    class MisalignmentCache {
    public:

      /** Hash map type to store existing misalignment transforms */
      typedef std::unordered_map<Belle2::VxdID::baseType, TGeoHMatrix> MisalignmentMap;

      /** destructor to clean up misalignment */
      ~MisalignmentCache() { clear(); };

      /** clear cache data */
      void clear() { m_misalignments.clear(); m_isAlive = false; }

      /** Read misalignment data from an xml file and store sensor misalignments.
       * @param filename name of the xml file */
      void readMisalignmentsFromXml(const std::string& filename);

      /** Add a new entry to the list of sensor misalignments.
       * This method will manually add a new (VxdID, Transform3D) pair
       * to the list.
       * @param misalignment 3D transform to add to the list of sensor misalignments.
       */
      void addMisalignment(Belle2::VxdID sensorID, TGeoHMatrix& misalignment)
      {
        m_misalignments[sensorID] = misalignment;
        m_isAlive = true;
      }

      /** Return the list of sensor misalignments */
      const MisalignmentMap& getMisalignments() const { return m_misalignments; }

      /** Return the misalignment transform for a given sensor.
       * @param id VxdID of the desired sensor
       * @return identity transform if no data found.
       */
      const TGeoHMatrix& getMisalignment(Belle2::VxdID id) const;

      /** Return the misalignment transform for a given sensor.
       * This function is a shorthand for MisalignmentCache::getInstance().getMisalignment
       */
      static const TGeoHMatrix& get(Belle2::VxdID id)
      { return getInstance().getMisalignment(id); }

      /** Return a reference to the singleton instance */
      static MisalignmentCache& getInstance();

      /** Return alive status of the cache (are there misalignment data? */
      static bool isAlive()
      { return getInstance().m_isAlive; }

    private:

      /** Singleton class, hidden constructor */
      MisalignmentCache() {};
      /** Singleton class, no copying */
      MisalignmentCache(const MisalignmentCache&) = delete;
      /** Singleton class, no assignment */
      MisalignmentCache& operator=(const MisalignmentCache&) = delete;

      /** Is the cache alive? */
      bool m_isAlive;
      /** Map to hold solid body misalignments for sensors. */
      MisalignmentMap m_misalignments;
    };
  }
} //Belle2 namespace
#endif
