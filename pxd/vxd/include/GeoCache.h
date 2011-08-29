/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_GEOCACHE_H
#define VXD_GEOCACHE_H

#include <framework/logging/Logger.h>

#include <pxd/vxd/VxdID.h>
#include <pxd/vxd/SensorInfoBase.h>
#include <set>
#include <map>
#include <boost/unordered_map.hpp>

class G4VPhysicalVolume;

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {
    /**
     * Class to faciliate easy access to sensor information of the VXD like
     * coordinate transformations or pitch size.
     */
    class GeoCache {
    public:
      /** desttructor to clean up structures */
      ~GeoCache() { clear(); };


      /** Clean up internal structures */
      void clear() {
        m_pxdLayers.clear();
        m_svdLayers.clear();
        m_ladders.clear();
        m_sensors.clear();
        m_sensorInfo.clear();
      }

      /** Search a given Geometry for Sensors.
       * This method will search a given Physical Volume and add all
       * sensitive Volumes where the SensitiveDetector implementation
       * inherits from vxd::SensitiveDetector to the cache.
       *
       * The given volume *must be* a volume placed directly in the top
       * volume, otherwise the transformations cannot be determined
       * correctly.
       *
       * This method should be called by the geometry creator once all
       * sensitive volumes are placed
       *
       * @param envelope Geometry tree to search, usually the Envelope of PXD or SVD
       */
      void findVolumes(G4VPhysicalVolume* envelope);

      /** Return a set of all known Layers.
       * @param sensortype Wether to only return the layer IDs for PXD, SVD or all VXD layers (default)
       * @return a set containing all existing Layers with the requested type
       */
      const std::set<VxdID>  getLayers(SensorInfoBase::SensorType sensortype = SensorInfoBase::VXD);
      /** Return a set of all ladder IDs belonging to a given layer */
      const std::set<VxdID>& getLadders(VxdID layer) const;
      /** Return a set of all sensor IDs belonging to a given ladder */
      const std::set<VxdID>& getSensors(VxdID ladder) const;

      /** Return a referecne to the SensorInfo of a given SensorID */
      const SensorInfoBase& getSensorInfo(VxdID id) const {
        SensorInfoMap::const_iterator info = m_sensorInfo.find(id);
        if (info == m_sensorInfo.end()) B2FATAL("VXD Sensor " << id << " does not exist.");
        return *(info->second);
      }

      /** Return a reference to the SensorInfo of a given SensorID.
       * This function is a shorthand for GeoCache::getInstance().getSensorInfo
       */
      static const SensorInfoBase& get(VxdID id) { return getInstance().getSensorInfo(id); }
      /** Return a reference to the singleton instance */
      static GeoCache& getInstance();
    private:
      /** Hash map to store pointers to all existing SensorInfos with constant lookup complexity */
      typedef boost::unordered_map<VxdID, SensorInfoBase*> SensorInfoMap;
      /** Map to store a set of unique VxdIDs belonging to one VxdID. Like ladders belong to layers, etc. */
      typedef std::map<VxdID, std::set<VxdID> > SensorHierachy;

      /** Singleton class, hidden constructor */
      GeoCache() {};
      /** Singleton class, hidden copy constructor */
      GeoCache(const GeoCache&);
      /** Singleton class, hidden assignment operator */
      GeoCache& operator=(const GeoCache&);

      /** Set of all PXD layer IDs */
      std::set<VxdID> m_pxdLayers;
      /** Set of all SVD layer IDs */
      std::set<VxdID> m_svdLayers;
      /** Map of all Ladder IDs belonging to a given Layer ID */
      SensorHierachy m_ladders;
      /** Map of all Sensor IDs belonging to a given Ladder ID */
      SensorHierachy m_sensors;
      /** Map to find the SensorInfo for a given Sensor ID */
      SensorInfoMap m_sensorInfo;
    };
  }
} //Belle2 namespace
#endif
