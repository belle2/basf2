/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <vxd/geometry/GeoCache.h>
#include <vxd/simulation/SensitiveDetectorBase.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <TMath.h>

#include <stack>
#include <memory>

#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4NavigationHistory.hh>
#include <G4Transform3D.hh>

using namespace std;

namespace Belle2 {
  namespace VXD {
    GeoCache::GeoCache()
    {
      // Add callback to update ReconstructionTransformation whenever alignment changes
      m_vxdAlignments.addCallback(this, &VXD::GeoCache::setupReconstructionTransformations);
    }

    void GeoCache::clear()
    {
      m_pxdLayers.clear();
      m_svdLayers.clear();
      m_telLayers.clear();
      m_ladders.clear();
      m_sensors.clear();
      m_sensorInfo.clear();

      m_halfShellPlacements.clear();
      m_ladderPlacements.clear();
      m_sensorPlacements.clear();
    }

    bool GeoCache::validSensorID(Belle2::VxdID id) const
    {
      id.setSegmentNumber(0);
      SensorInfoMap::const_iterator info = m_sensorInfo.find(id);
      return (info != m_sensorInfo.end());
    }

    const vector<VxdID> GeoCache::getListOfSensors() const
    {
      vector<VxdID> sensors;
      for (auto entry : m_sensorInfo)
        sensors.push_back(entry.first);
      return sensors;
    }

    const SensorInfoBase& GeoCache::getSensorInfo(VxdID id) const
    {
      id.setSegmentNumber(0);
      SensorInfoMap::const_iterator info = m_sensorInfo.find(id);
      if (info == m_sensorInfo.end()) {
        B2WARNING("VXD Sensor " << id << " does not exist.");
        return *(m_sensorInfo.begin()->second);
      }
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
          info->setTransformation(transform); // cppcheck-suppress nullPointerRedundantCheck
          info->setTransformation(transform, true); // cppcheck-suppress nullPointerRedundantCheck

          addSensor(info);
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

      // Finally set-up reconstruction transformations
      setupReconstructionTransformations();
    }

    void GeoCache::addSensor(SensorInfoBase* sensorinfo)
    {
      //Save pointer to the SensorInfo and update lists of all existing
      //layers,ladders,sensors
      VxdID sensorID = sensorinfo->getID();
      VxdID ladderID = sensorID;
      ladderID.setSensorNumber(0);
      VxdID layerID  = ladderID;
      layerID.setLadderNumber(0);

      m_sensorInfo[sensorID] = sensorinfo;

      switch (sensorinfo->getType()) {
        case SensorInfoBase::PXD:
          m_pxdLayers.insert(layerID);
          break;
        case SensorInfoBase::SVD:
          m_svdLayers.insert(layerID);
          break;
        case SensorInfoBase::TEL:
          m_telLayers.insert(layerID);
          break;
        default:
          B2FATAL("Cannot use anything else as SensorTypes PXD, SVD, or TEL when creating VXD Sensors");
      }
      m_ladders[layerID].insert(ladderID);
      m_sensors[ladderID].insert(sensorID);
    }

    const set<VxdID> GeoCache::getLayers(SensorInfoBase::SensorType type)
    {
      switch (type) {
        case SensorInfoBase::PXD:
          return m_pxdLayers;
        case SensorInfoBase::SVD:
          return m_svdLayers;
        case SensorInfoBase::TEL:
          return m_telLayers;
        default:
          std::set<VxdID> allLayers = m_pxdLayers;
          allLayers.insert(m_svdLayers.begin(), m_svdLayers.end());
          allLayers.insert(m_telLayers.begin(), m_telLayers.end());
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
      static unique_ptr<GeoCache> instance(new GeoCache());
      return *instance;
    }

    void GeoCache::addSensorPlacement(VxdID ladder, VxdID sensor, const G4Transform3D& placement)
    {
      m_sensorPlacements[ladder].push_back(std::make_pair(sensor, g4Transform3DToTGeo(placement)));
    }

    void GeoCache::addLadderPlacement(VxdID halfShell, VxdID ladder, const G4Transform3D& placement)
    {
      m_ladderPlacements[halfShell].push_back(std::make_pair(ladder, g4Transform3DToTGeo(placement)));
      // Add the (empty) container for sensor placements inside this ladder
      m_sensorPlacements[ladder] = std::vector<std::pair<VxdID, TGeoHMatrix>>();
    }

    void GeoCache::addHalfShellPlacement(VxdID halfShell, const G4Transform3D& placement)
    {
      m_halfShellPlacements[halfShell] = g4Transform3DToTGeo(placement);
      // Add the (empty) container for ladder placements inside this halfshell
      m_ladderPlacements[halfShell] = std::vector<std::pair<VxdID, TGeoHMatrix>>();
    }

    const map<VxdID, TGeoHMatrix>& GeoCache::getHalfShellPlacements() const {return m_halfShellPlacements;}

    const vector< pair< VxdID, TGeoHMatrix > >& GeoCache::getSensorPlacements(VxdID ladder) const
    {
      auto placements = m_sensorPlacements.find(ladder);
      if (placements == m_sensorPlacements.end())
        throw std::invalid_argument("Invalid ladder id " + (std::string) ladder);

      return placements->second;
    }

    const vector< pair< VxdID, TGeoHMatrix > >& GeoCache::getLadderPlacements(VxdID halfShell) const
    {
      auto placements = m_ladderPlacements.find(halfShell);
      if (placements == m_ladderPlacements.end())
        throw std::invalid_argument("Invalid half-shelve id " + (std::string) halfShell);

      return placements->second;
    }

    void GeoCache::setupReconstructionTransformations()
    {
      if (!m_vxdAlignments.isValid()) {
        B2WARNING("No VXD alignment data. Defaults (0's) will be used!");
        return;
      }

      // Loop over VXD sensors to read parameters for description of planar defomration
      for (auto& sensorID : getListOfSensors()) {
        std::vector<double> planarParameters = {
          // Numbering of VXD alignment parameters:
          //  -> 0-6:   Rigid body alignment
          //  -> 31-33: First level of surface deformation
          //  -> 41-44: Second level of surface deformation
          //  -> 51-55: Third level of surface deformation
          m_vxdAlignments->get(sensorID, 31),
          m_vxdAlignments->get(sensorID, 32),
          m_vxdAlignments->get(sensorID, 33),
          m_vxdAlignments->get(sensorID, 41),
          m_vxdAlignments->get(sensorID, 42),
          m_vxdAlignments->get(sensorID, 43),
          m_vxdAlignments->get(sensorID, 44),
          m_vxdAlignments->get(sensorID, 51),
          m_vxdAlignments->get(sensorID, 52),
          m_vxdAlignments->get(sensorID, 53),
          m_vxdAlignments->get(sensorID, 54),
          m_vxdAlignments->get(sensorID, 55),
        };

        // Store parameters for planar deformation
        VXD::SensorInfoBase& sensorInfo = const_cast<VXD::SensorInfoBase&>(getSensorInfo(sensorID));
        sensorInfo.setSurfaceParameters(planarParameters);
      }

      /**
      So the hierarchy is as follows:
                  Belle 2
                / |     | \
           Ying  Yang Pat  Mat ... other sub-detectors
           / |   / |  |  \  | \
          ......  ladders ......
         / / |   / |  |  \  | \ \
        ......... sensors ........
      */

      for (auto& halfShellPlacement : getHalfShellPlacements()) {
        TGeoHMatrix trafoHalfShell = halfShellPlacement.second;
        trafoHalfShell *= getTGeoFromRigidBodyParams(
                            m_vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dU),
                            m_vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dV),
                            m_vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dW),
                            m_vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dAlpha),
                            m_vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dBeta),
                            m_vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dGamma)
                          );

        for (auto& ladderPlacement : getLadderPlacements(halfShellPlacement.first)) {
          // Updated trafo
          TGeoHMatrix trafoLadder = ladderPlacement.second;
          trafoLadder *= getTGeoFromRigidBodyParams(
                           m_vxdAlignments->get(ladderPlacement.first, VXDAlignment::dU),
                           m_vxdAlignments->get(ladderPlacement.first, VXDAlignment::dV),
                           m_vxdAlignments->get(ladderPlacement.first, VXDAlignment::dW),
                           m_vxdAlignments->get(ladderPlacement.first, VXDAlignment::dAlpha),
                           m_vxdAlignments->get(ladderPlacement.first, VXDAlignment::dBeta),
                           m_vxdAlignments->get(ladderPlacement.first, VXDAlignment::dGamma)
                         );

          for (auto& sensorPlacement : getSensorPlacements(ladderPlacement.first)) {
            // Updated trafo
            TGeoHMatrix trafoSensor = sensorPlacement.second;
            trafoSensor *= getTGeoFromRigidBodyParams(
                             m_vxdAlignments->get(sensorPlacement.first, VXDAlignment::dU),
                             m_vxdAlignments->get(sensorPlacement.first, VXDAlignment::dV),
                             m_vxdAlignments->get(sensorPlacement.first, VXDAlignment::dW),
                             m_vxdAlignments->get(sensorPlacement.first, VXDAlignment::dAlpha),
                             m_vxdAlignments->get(sensorPlacement.first, VXDAlignment::dBeta),
                             m_vxdAlignments->get(sensorPlacement.first, VXDAlignment::dGamma)
                           );

            // Store new reco-transformation
            VXD::SensorInfoBase& geometry = const_cast<VXD::SensorInfoBase&>(getSensorInfo(sensorPlacement.first));
            geometry.setTransformation(trafoHalfShell * trafoLadder * trafoSensor, true);

          }
        }
      }

    }

    TGeoHMatrix GeoCache::g4Transform3DToTGeo(const G4Transform3D& g4)
    {
      TGeoHMatrix trafo;
      // Differential translation
      TGeoTranslation translation;
      // Differential rotation
      TGeoRotation rotation;

      //NOTE: for some reason, there is cm vs. mm difference
      trafo.SetDx(g4.getTranslation()[0] / 10.);
      trafo.SetDy(g4.getTranslation()[1] / 10.);
      trafo.SetDz(g4.getTranslation()[2] / 10.);

      Double_t rotMatrix[9];
      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
          rotMatrix[i * 3 + j] = g4.getRotation()[i][j];
        }
      }
      trafo.SetRotation(rotMatrix);
      return trafo;
    }

    TGeoHMatrix GeoCache::getTGeoFromRigidBodyParams(double dU, double dV, double dW, double dAlpha, double dBeta, double dGamma)
    {
      // Differential translation
      TGeoTranslation translation;
      // Differential rotation
      TGeoRotation rotation;

      translation.SetTranslation(dU, dV, dW);
      rotation.RotateX(- dAlpha * TMath::RadToDeg());
      rotation.RotateY(- dBeta  * TMath::RadToDeg());
      rotation.RotateZ(- dGamma * TMath::RadToDeg());

      // Differential trafo (trans + rot)
      TGeoCombiTrans combi(translation, rotation);
      TGeoHMatrix trafo;
      trafo = trafo * combi;
      return trafo;
    }
  } //VXD namespace
} //Belle2 namespace
