/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/GlobalParam.h>

#include <alignment/Hierarchy.h>
#include <cdc/dbobjects/CDCLayerAlignment.h>
#include <framework/database/Database.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>

#include <map>
#include <string>
#include <vector>

namespace Belle2 {
  namespace alignment {
    //std::tuple<BeamSpot, VXDAlignment, CDCAlignment, CDCLayerAlignment, CDCTimeWalks, CDCTimeZeros, CDCXtRelations, BKLMAlignment, EKLMAlignment> dbvector = {};

    /// The DB object unique id in global calibration
    template <>
    unsigned short GlobalParamSet<BeamSpot>::getGlobalUniqueID() const { return 1; }
    /// Get global parameter of the DB object by its element and parameter number
    /// Note this is not const, it might need to construct the object
    template <>
    double GlobalParamSet<BeamSpot>::getGlobalParam(unsigned short element, unsigned short param)
    {
      this->ensureConstructed();
      if (element != 0 or param > 3) {
        B2ERROR("Invalid global BeamSpot parameter id");
        return 0;
      }

      if (auto bp = dynamic_cast<BeamSpot*>(this->getDBObj()))
        return bp->getIPPosition()[param - 1];

      B2ERROR("Could not get value for BeamSpot");
      return 0.;
    }
    /// Set global parameter of the DB object by its element and parameter number
    template <>
    void GlobalParamSet<BeamSpot>::setGlobalParam(double value, unsigned short element, unsigned short param)
    {
      this->ensureConstructed();
      if (element != 0 or param > 3) {
        B2ERROR("Invalid global BeamSpot id");
        return;
      }
      if (auto bp = dynamic_cast<BeamSpot*>(this->getDBObj())) {
        B2Vector3D vertex = bp->getIPPosition();
        vertex[param - 1] = value;
        bp->setIP(vertex, bp->getIPPositionCovMatrix());
      } else {
        B2ERROR("Could not set value for BeamSpot");
      }

    }
    /// List global parameters in this DB object
    template <>
    std::vector<std::pair<unsigned short, unsigned short>> GlobalParamSet<BeamSpot>::listGlobalParams()
    {
      return {{0, 1}, {0, 2}, {0, 3}};
    }


    GlobalParamVector::GlobalParamVector(const std::vector< std::string >& components) : m_components(components) {}

    void GlobalParamVector::updateGlobalParam(double difference, short unsigned int uniqueID, short unsigned int element,
                                              short unsigned int param)
    {
      // TODO: niw can be replaced by other fcn updateGlobalParam of GlobalParamSetAccess
      auto prev = getGlobalParam(uniqueID, element, param);
      setGlobalParam(prev + difference, uniqueID, element, param);
    }
    void GlobalParamVector::setGlobalParam(double value, short unsigned int uniqueID, short unsigned int element,
                                           short unsigned int param)
    {
      auto dbObj = m_vector.find(uniqueID);
      if (dbObj != m_vector.end()) {
        dbObj->second->setGlobalParam(value, element, param);
      } else {
        B2WARNING("Did not found DB object with unique id " << uniqueID << " in global vector. Cannot set value for element " << element <<
                  " and parameter " << param);
      }
    }
    double GlobalParamVector::getGlobalParam(short unsigned int uniqueID, short unsigned int element, short unsigned int param)
    {
      auto dbObj = m_vector.find(uniqueID);
      if (dbObj != m_vector.end()) {
        return dbObj->second->getGlobalParam(element, param);
      } else {
        B2WARNING("Did not found DB object with unique id " << uniqueID << " in global vector. Cannot get value for element " << element <<
                  " and parameter " << param << ". Returning 0.");
        return 0.;
      }
    }
    std::vector< std::tuple< short unsigned int, short unsigned int, short unsigned int > > GlobalParamVector::listGlobalParams()
    {
      std::vector<std::tuple<unsigned short, unsigned short, unsigned short>> params;
      for (auto& uID_DBObj : m_vector) {
        for (auto element_param : uID_DBObj.second->listGlobalParams()) {
          params.push_back({uID_DBObj.first, element_param.first, element_param.second});
        }
      }
      return params;
    }
    std::vector< TObject* > GlobalParamVector::releaseObjects(bool onlyConstructed)
    {
      std::vector<TObject*> result;
      for (auto& uID_DBObj : m_vector) {
        if (onlyConstructed and not uID_DBObj.second->isConstructed())
          continue;

        result.push_back({uID_DBObj.second->releaseObject()});
      }
      return result;
    }
    void GlobalParamVector::loadFromDB()
    {
      for (auto& uID_DBObj : m_vector) {
        uID_DBObj.second->loadFromDBObjPtr();
      }
    }
    void GlobalParamVector::loadFromDB(const EventMetaData& event)
    {
      for (auto& uID_DBObj : m_vector) {
        uID_DBObj.second->loadFromDB(event);
      }
    }
    void GlobalParamVector::postHierarchyChanged(GlobalDerivativesHierarchy& hierarchy)
    {
      for (auto interface : m_subDetectorInterfacesVector) {
        interface->setupAlignmentHierarchy(hierarchy);
      }

    }
    void GlobalParamVector::construct()
    {
      for (auto& uID_DBObj : m_vector) {
        if (not uID_DBObj.second->isConstructed()) uID_DBObj.second->construct();
      }
    }
    void GlobalParamVector::postReadFromResult(
      std::vector< std::tuple< short unsigned int, short unsigned int, short unsigned int, double > >& result)
    {
      for (auto interface : m_subDetectorInterfacesVector) {
        interface->readFromResult(result, *this);
      }
    }

    VXDGlobalParamInterface::E_VXDHierarchyType VXDGlobalParamInterface::s_hierarchyType =
      VXDGlobalParamInterface::E_VXDHierarchyType::c_Full;
    bool VXDGlobalParamInterface::s_enablePXD = true;
    bool VXDGlobalParamInterface::s_enableSVD = true;

    void VXDGlobalParamInterface::setupAlignmentHierarchy(GlobalDerivativesHierarchy& hierarchy)
    {
      try {
        auto& rigidBodyHierarchy = dynamic_cast<RigidBodyHierarchy&>(hierarchy);
        auto& geo = VXD::GeoCache::getInstance();
        // Set-up hierarchy
        DBObjPtr<VXDAlignment> vxdAlignments;
        if (s_hierarchyType == c_None) {
          // Nothing to be done
          return;

        } else if (s_hierarchyType == c_Flat) {
          for (auto& sensor : geo.getListOfSensors()) {
            //TODO: Don't we have better way to distinguish PXD and SVD?
            // For PXD
            if (sensor.getLayerNumber() <= 2 and !s_enablePXD)
              continue;
            // SVD layer > 2
            if (sensor.getLayerNumber() > 2 and !s_enableSVD)
              continue;

            rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(sensor, VxdID(0, 0, 0, 1),
                geo.get(sensor).getTransformation(true));
          }
        } else if (s_hierarchyType == c_HalfShells) {
          /**
          This simpler hierarchy is as follows:
                      Belle 2
                    / |     | \
                Ying  Yang Pat  Mat ... other sub-detectors
              / / |   / |  |  \  | \ \
            ......... sensors ........
          */

          for (auto& halfShellPlacement : geo.getHalfShellPlacements()) {
            // For PXD
            if (halfShellPlacement.first.getLayerNumber() <= 2 and !s_enablePXD)
              continue;
            // for SVD - layer > 2
            if (halfShellPlacement.first.getLayerNumber() > 2 and !s_enableSVD)
              continue;

            TGeoHMatrix trafoHalfShell = halfShellPlacement.second;
            trafoHalfShell *= geo.getTGeoFromRigidBodyParams(
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dU),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dV),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dW),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dAlpha),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dBeta),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dGamma)
                              );
            rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, alignment::EmptyGlobalParamSet>(halfShellPlacement.first, 0, trafoHalfShell);

            for (auto& ladderPlacement : geo.getLadderPlacements(halfShellPlacement.first)) {
              // Updated trafo
              TGeoHMatrix trafoLadder = ladderPlacement.second;
              trafoLadder *= geo.getTGeoFromRigidBodyParams(
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dU),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dV),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dW),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dAlpha),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dBeta),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dGamma)
                             );
              // Do not insert ladder, but insert sensor directly into half-shells bellow, with the additional transformation from ladder
              //rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(ladderPlacement.first, halfShellPlacement.first, trafoLadder);

              for (auto& sensorPlacement : geo.getSensorPlacements(ladderPlacement.first)) {
                // Updated trafo
                TGeoHMatrix trafoSensor = sensorPlacement.second;
                trafoSensor *= geo.getTGeoFromRigidBodyParams(
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dU),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dV),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dW),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dAlpha),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dBeta),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dGamma)
                               );
                rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(sensorPlacement.first, halfShellPlacement.first,
                    trafoLadder * trafoSensor);


              }
            }
          }
        } else if (s_hierarchyType == c_Full) {
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

          for (auto& halfShellPlacement : geo.getHalfShellPlacements()) {
            // For PXD
            if (halfShellPlacement.first.getLayerNumber() <= 2 and !s_enablePXD)
              continue;
            // for SVD - layer > 2
            if (halfShellPlacement.first.getLayerNumber() > 2 and !s_enableSVD)
              continue;

            TGeoHMatrix trafoHalfShell = halfShellPlacement.second;
            trafoHalfShell *= geo.getTGeoFromRigidBodyParams(
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dU),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dV),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dW),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dAlpha),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dBeta),
                                vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dGamma)
                              );
            rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, alignment::EmptyGlobalParamSet>(halfShellPlacement.first, 0, trafoHalfShell);

            for (auto& ladderPlacement : geo.getLadderPlacements(halfShellPlacement.first)) {
              // Updated trafo
              TGeoHMatrix trafoLadder = ladderPlacement.second;
              trafoLadder *= geo.getTGeoFromRigidBodyParams(
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dU),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dV),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dW),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dAlpha),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dBeta),
                               vxdAlignments->get(ladderPlacement.first, VXDAlignment::dGamma)
                             );
              rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(ladderPlacement.first, halfShellPlacement.first, trafoLadder);

              for (auto& sensorPlacement : geo.getSensorPlacements(ladderPlacement.first)) {
                // Updated trafo
                TGeoHMatrix trafoSensor = sensorPlacement.second;
                trafoSensor *= geo.getTGeoFromRigidBodyParams(
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dU),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dV),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dW),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dAlpha),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dBeta),
                                 vxdAlignments->get(sensorPlacement.first, VXDAlignment::dGamma)
                               );
                rigidBodyHierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(sensorPlacement.first, ladderPlacement.first, trafoSensor);


              }
            }
          }
        }
      } catch (const std::bad_cast& e) {
        // Bad cast
      }
    }

  }
}
