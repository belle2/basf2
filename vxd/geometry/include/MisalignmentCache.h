/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VXD_MISALIGNMENT_CACHE_H
#define VXD_MISALIGNMENT_CACHE_H

#include <vxd/dataobjects/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>
#include <unordered_map>
#include <tuple>

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
      /** Misalignment shift information contains a validity flag (if false, the misaligned object falls outside its sensor) and shifts in u and v.*/
      typedef std::tuple<bool, double, double> MisalignmentShiftType;

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
       * @param sensorID Sensor identifier.
       * @param misalignment 3D transform to add to the list of sensor misalignments.
       */
      void addMisalignment(Belle2::VxdID sensorID, const TGeoHMatrix& misalignment)
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
      const TGeoHMatrix& getMisalignmentTransform(Belle2::VxdID id) const;

      /** Return misalignment shift for a VXDTrueHit.
       * This is the principal misalignment method, used to misalign TrueHits or Clusters (via relation to their TrueHits.
       * @param hit pointer to the TrueHit to misalign
       * @return std::pair<double, double> of misalignment shifts
       **/
      MisalignmentShiftType getMisalignmentShift(const VXDTrueHit* hit);

      /** Return misalignment shift for a VXDTrueHit.
       * This is a shorthand for MisalignmentCache::getInstance().getMisalignmentShift.
       */
      static MisalignmentShiftType getMisalignment(const VXDTrueHit* hit)
      { return getInstance().getMisalignmentShift(hit); }

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
      bool m_isAlive{false};
      /** Map to hold solid body misalignments for sensors. */
      MisalignmentMap m_misalignments;
    };
  }
} //Belle2 namespace
#endif
