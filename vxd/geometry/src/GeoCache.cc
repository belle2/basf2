/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <vxd/geometry/GeoCache.h>
#include <vxd/simulation/SensitiveDetectorBase.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <stack>
#include <memory>

#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4NavigationHistory.hh>

using namespace std;

namespace Belle2 {
  namespace VXD {
    void GeoCache::clear()
    {
      m_pxdLayers.clear();
      m_svdLayers.clear();
      m_ladders.clear();
      m_sensors.clear();
      m_sensorInfo.clear();
    }
    const SensorInfoBase& GeoCache::getSensorInfo(VxdID id) const
    {
      id.setSegmentNumber(0);
      SensorInfoMap::const_iterator info = m_sensorInfo.find(id);
      if (info == m_sensorInfo.end()) B2FATAL("VXD Sensor " << id << " does not exist.");
      return *(info->second);
    }

    void GeoCache::findVolumes(G4VPhysicalVolume* envelope)
    {
      //So, lets loop over the geometry tree and find all sensitive volumes.
      //To get the correct Transformation matrices we use the
      //G4NavigationHistory which is a stack of premultiplied transformations.
      G4NavigationHistory nav;
      //To avoid recursion we store all volumes in a stack, starting with the
      //envelope we are about to search in
      stack<G4VPhysicalVolume*> volumes;
      volumes.push(envelope);

      //Now lets just continue until the stack is empty
      while (!volumes.empty()) {
        G4VPhysicalVolume* physical = volumes.top();
        volumes.pop();
        //NULL indicates that we are finished with the children of a node, so
        //pop a transformation level
        if (!physical) {
          nav.BackLevel();
          continue;
        }
        //Add a NULL to so that we know that all children are finished once we
        //emptied the stack of volumes back to this NULL
        volumes.push(0);
        //Now enter the volume
        nav.NewLevel(physical);

        G4LogicalVolume* logical = physical->GetLogicalVolume();

        //Check if we found a sensitive volume with correct type
        SensitiveDetectorBase* sensitive = dynamic_cast<SensitiveDetectorBase*>(logical->GetSensitiveDetector());
        if (sensitive) {
          //Apparently we did, so get the sensor Information and add it
          SensorInfoBase* info = sensitive->getSensorInfo();
          if (!info) B2FATAL("No SensorInfo found for Volume " << logical->GetName());

          //Convert transformation to ROOT
          const G4AffineTransform g4transform = nav.GetTopTransform().Inverse();
          TGeoHMatrix transform;
          double rotation[9] = {
            g4transform[0], g4transform[4], g4transform[8],
            g4transform[1], g4transform[5], g4transform[9],
            g4transform[2], g4transform[6], g4transform[10]
          };
          transform.SetRotation(rotation);
          transform.SetDx(g4transform[12]*Unit::mm);
          transform.SetDy(g4transform[13]*Unit::mm);
          transform.SetDz(g4transform[14]*Unit::mm);
          info->setTransformation(transform);

          //Save pointer to the SensorInfo and update lists of all existing
          //layers,ladders,sensors
          VxdID sensorID = info->getID();
          VxdID ladderID = sensorID;
          ladderID.setSensorNumber(0);
          VxdID layerID  = ladderID;
          layerID.setLadderNumber(0);

          m_sensorInfo[sensorID] = info;

          switch (info->getType()) {
            case SensorInfoBase::PXD:
              m_pxdLayers.insert(layerID);
              break;
            case SensorInfoBase::SVD:
              m_svdLayers.insert(layerID);
              break;
            default:
              B2FATAL("Cannot use anything else as SensorType PXD or SVD when creating VXD Sensors");
          }
          m_ladders[layerID].insert(ladderID);
          m_sensors[ladderID].insert(sensorID);
        }

        int nDaughters = logical->GetNoDaughters();
        //Add all children. Since we use a stack they will be processed in
        //opposite order In principle we do not care, but for niceness sake we
        //add them back to front so that they are processed in the "correct"
        //order
        for (int i = nDaughters - 1; i >= 0; --i) {
          G4VPhysicalVolume* daughter = logical->GetDaughter(i);
          volumes.push(daughter);
        }
      }
    }

    const set<VxdID> GeoCache::getLayers(SensorInfoBase::SensorType type)
    {
      switch (type) {
        case SensorInfoBase::PXD:
          return m_pxdLayers;
        case SensorInfoBase::SVD:
          return m_svdLayers;
        default:
          std::set<VxdID> allLayers = m_pxdLayers;
          allLayers.insert(m_svdLayers.begin(), m_svdLayers.end());
          return allLayers;
      }
    }

    const set<VxdID>& GeoCache::getLadders(VxdID layer) const
    {
      //We only index by layer, so set everything else to 0
      layer.setLadderNumber(0);
      layer.setSensorNumber(0);
      layer.setSegmentNumber(0);
      SensorHierachy::const_iterator info = m_ladders.find(layer);
      if (info == m_ladders.end()) B2FATAL("VXD Layer " << layer << "does not exist.");
      return info->second;
    }

    const set<VxdID>& GeoCache::getSensors(VxdID ladder) const
    {
      //We only index by layer and ladder, set sensor to 0
      ladder.setSensorNumber(0);
      ladder.setSegmentNumber(0);
      SensorHierachy::const_iterator info = m_sensors.find(ladder);
      if (info == m_sensors.end()) B2FATAL("VXD Ladder " << ladder << "does not exist.");
      return info->second;
    }

    GeoCache& GeoCache::getInstance()
    {
      static auto_ptr<GeoCache> instance(new GeoCache());
      return *instance;
    }
  }
} //Belle2 namespace
