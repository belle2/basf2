/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/GlobalParam.h>

#include <string>
#include <vector>
#include <map>

#include <framework/logging/Logger.h>

#include <framework/database/Database.h>

#include <vxd/geometry/GeoCache.h>
#include <alignment/Hierarchy.h>

#include <cdc/dbobjects/CDCLayerAlignment.h>

namespace Belle2 {
  namespace alignment {
    //std::tuple<BeamParameters, VXDAlignment, CDCAlignment, CDCLayerAlignment, CDCTimeWalks, CDCTimeZeros, CDCXtRelations, BKLMAlignment, EKLMAlignment> dbvector = {};

    /// The DB object unique id in global calibration
    template <>
    unsigned short GlobalParamSet<BeamParameters>::getGlobalUniqueID() const { return 1; }
    /// Get global parameter of the DB object by its element and parameter number
    /// Note this is not const, it might need to construct the object
    template <>
    double GlobalParamSet<BeamParameters>::getGlobalParam(unsigned short element, unsigned short param)
    {
      this->ensureConstructed();
      if (element != 0 or param > 3) {
        B2ERROR("Invalid global BeamParameters parameter id");
        return 0;
      }

      if (auto bp = dynamic_cast<BeamParameters*>(this->getDBObj()))
        return bp->getVertex()[param - 1];

      B2ERROR("Could not get value for BeamParameters");
      return 0.;
    }
    /// Set global parameter of the DB object by its element and parameter number
    template <>
    void GlobalParamSet<BeamParameters>::setGlobalParam(double value, unsigned short element, unsigned short param)
    {
      this->ensureConstructed();
      if (element != 0 or param > 3) {
        B2ERROR("Invalid global BeamParameters id");
        return;
      }
      if (auto bp = dynamic_cast<BeamParameters*>(this->getDBObj())) {
        TVector3 vertex = bp->getVertex();
        vertex[param - 1] = value;
        bp->setVertex(vertex);
      } else {
        B2ERROR("Could not set value for BeamParameters");
      }

    }
    /// List global parameters in this DB object
    template <>
    std::vector<std::pair<unsigned short, unsigned short>> GlobalParamSet<BeamParameters>::listGlobalParams()
    {
      return {{0, 1}, {0, 2}, {0, 3}};
    }


    GlobalParamVector::GlobalParamVector(std::vector< std::string > components) : m_components(components) {}

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
    void CDCGlobalParamInterface::readFromResult(
      std::vector<std::tuple<short unsigned int, short unsigned int, short unsigned int, double> >& result,
      Belle2::alignment::GlobalParamVector& thisGlobalParamVector)
    {
      std::cout << "Reading from Pede result into CDCAlignment..." << std::endl;
      std::vector<double> dxLayer(56, 0.);
      std::vector<double> dyLayer(56, 0.);
      std::vector<double> dPhiLayer(56, 0.);

      for (auto entry : result) {
        unsigned short uid = std::get<0>(entry);
        if (uid != CDCLayerAlignment::getGlobalUniqueID())
          continue;

        unsigned short layer = std::get<1>(entry);
        unsigned short param = std::get<2>(entry);
        double correction = std::get<3>(entry);

        if (param == 1)
          dxLayer[layer] = correction;
        if (param == 2)
          dyLayer[layer] = correction;
        if (param == 6)
          dPhiLayer[layer] = correction;

      }

//      if (auto cdcLayerAlignment = thisGlobalParamVector.getDBObj<CDCLayerAlignment>()) {
      if (auto cdc = thisGlobalParamVector.getDBObj<CDCAlignment>()) {

        std::vector<double> R = {
          168.0, 178.0, 188.0, 198.0, 208.0, 218.0, 228.0, 238.0,
          257.0, 275.2, 293.4, 311.6, 329.8, 348.0,
          365.2, 383.4, 401.6, 419.8, 438.0, 455.7,
          476.9, 494.6, 512.8, 531.0, 549.2, 566.9,
          584.1, 601.8, 620.0, 638.2, 656.4, 674.1,
          695.3, 713.0, 731.2, 749.4, 767.6, 785.3,
          802.5, 820.2, 838.4, 856.6, 874.8, 892.5,
          913.7, 931.4, 949.6, 967.8, 986.0, 1003.7,
          1020.9, 1038.6, 1056.8, 1075.0, 1093.2, 1111.4
        };

        std::vector<int> nWires = {
          160, 160, 160, 160, 160, 160, 160, 160,
          160, 160, 160, 160, 160, 160,
          192, 192, 192, 192, 192, 192,
          224, 224, 224, 224, 224, 224,
          256, 256, 256, 256, 256, 256,
          288, 288, 288, 288, 288, 288,
          320, 320, 320, 320, 320, 320,
          352, 352, 352, 352, 352, 352,
          384, 384, 384, 384, 384, 384
        };

        std::vector<int> shiftHold = {
          0, 0, 0, 0, 0, 0, 0, 0,
          18, 18, 18, 18, 18, 18,
          0, 0, 0, 0, 0, 0,
          -18, -18, -18, -18, -18, -18,
          0, 0, 0, 0, 0, 0,
          19, 19, 19, 19, 19, 19,
          0, 0, 0, 0, 0, 0,
          -20, -20, -20, -20, -20, -20,
          0, 0, 0, 0, 0, 0
        };
        std::vector<double> offset = {
          0.5, 0., 0.5, 0., 0.5, 0.,  0.5, 0.,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5,
          0., 0.5, 0., 0.5, 0.,  0.5
        };

//           std::vector<double> rotLayer(56, 0.);
//           std::vector<double> xLayer(56, 0.);
//           std::vector<double> yLayer(56, 0.);
//
//           for (int l = 0; l < 56; ++l) {
//             rotLayer.at(l) = cdcLayerAlignment->getGlobalParam(l, CDCLayerAlignment::layerPhi);
//             xLayer.at(l) = cdcLayerAlignment->getGlobalParam(l, CDCLayerAlignment::layerX);
//             yLayer.at(l) = cdcLayerAlignment->getGlobalParam(l, CDCLayerAlignment::layerY);
//           }




        std::vector<std::vector<std::vector<double>>> misBackward(56, std::vector<std::vector<double>>(385, std::vector<double>(3, 0.)));
        std::vector<std::vector<std::vector<double>>> misForward(56, std::vector<std::vector<double>>(385, std::vector<double>(3, 0.)));

        double PhiF, PhiB;
        for (int l = 0; l < 56; ++l) {
          double dPhi = 2 * 3.14159265358979323846 / nWires[l];
          for (int w = 0; w < nWires[l]; ++w) {

            PhiF = dPhi * (w + offset[l] + 0.5 * shiftHold[l]);

            double xWire = R[l] * 0.1 * cos(PhiF) + cdc->get(WireID(l, w), CDCAlignment::wireFwdX);
            double yWire = R[l] * 0.1 * sin(PhiF) + cdc->get(WireID(l, w), CDCAlignment::wireFwdY);

            misForward[l][w][0] = dxLayer[l] + (1. - cos(dPhiLayer[l])) * xWire + sin(dPhiLayer[l]) * yWire;
            misForward[l][w][1] = dyLayer[l] + (1. - cos(dPhiLayer[l])) * yWire - sin(dPhiLayer[l]) * xWire;
            misForward[l][w][2] = 0. ;

            PhiB = dPhi * (w + offset[l]);

            xWire = R[l] * 0.1 * cos(PhiB) + cdc->get(WireID(l, w), CDCAlignment::wireBwdX);
            yWire = R[l] * 0.1 * sin(PhiB) + cdc->get(WireID(l, w), CDCAlignment::wireBwdY);

            misBackward[l][w][0] = dxLayer[l] + (1. - cos(dPhiLayer[l])) * xWire + sin(dPhiLayer[l]) * yWire;
            misBackward[l][w][1] = dyLayer[l] + (1. - cos(dPhiLayer[l])) * yWire - sin(dPhiLayer[l]) * xWire;
            misBackward[l][w][2] = 0.;


            /*PhiF = dPhi * (w + offset[l] + 0.5 * shiftHold[l]);
            misForward[l][w][0] = xLayer[l] + R[l] * 0.1 * (cos(PhiF + rotLayer[l]) - cos(PhiF));
            misForward[l][w][1] = yLayer[l] + R[l] * 0.1 * (sin(PhiF + rotLayer[l]) - sin(PhiF));
            misForward[l][w][2] = 0. ;

            PhiB = dPhi * (w + offset[l]);
            misBackward[l][w][0] = xLayer[l] + R[l] * 0.1 * (cos(PhiB + rotLayer[l]) - cos(PhiB));
            misBackward[l][w][1] = yLayer[l] + R[l] * 0.1 * (sin(PhiB + rotLayer[l]) - sin(PhiB));
            misBackward[l][w][2] = 0.;*/
          }
        }

        for (int l = 0; l < 56; ++l) {
          for (int w = 0; w < nWires[l]; ++w) {

            auto wire = WireID(l, w);

            cdc->add(wire, CDCAlignment::wireFwdX, misForward[l][w][0]);
            cdc->add(wire, CDCAlignment::wireFwdY, misForward[l][w][1]);
            cdc->add(wire, CDCAlignment::wireFwdZ, misForward[l][w][2]);

            cdc->add(wire, CDCAlignment::wireBwdX, misBackward[l][w][0]);
            cdc->add(wire, CDCAlignment::wireBwdY, misBackward[l][w][1]);
            cdc->add(wire, CDCAlignment::wireBwdZ, misBackward[l][w][2]);

            /*cdc->set(wire, CDCAlignment::wireFwdX, misForward[l][w][0] + cdc->get(wire, CDCAlignment::wireFwdX));
            cdc->set(wire, CDCAlignment::wireFwdY, misForward[l][w][1] + cdc->get(wire, CDCAlignment::wireFwdY));
            cdc->set(wire, CDCAlignment::wireFwdZ, misForward[l][w][2] + cdc->get(wire, CDCAlignment::wireFwdZ));

            cdc->set(wire, CDCAlignment::wireBwdX, misBackward[l][w][0] + cdc->get(wire, CDCAlignment::wireBwdX));
            cdc->set(wire, CDCAlignment::wireBwdY, misBackward[l][w][1] + cdc->get(wire, CDCAlignment::wireBwdY));
            cdc->set(wire, CDCAlignment::wireBwdZ, misBackward[l][w][2] + cdc->get(wire, CDCAlignment::wireBwdZ)); */

            //NOTE: Cannot do following - CDCAlignment ignorres setGlobalParam/getGlobalParam
            // thisGlobalParamVector.updateGlobalParam(misForward[l][w][0], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireFwdX);
            // thisGlobalParamVector.updateGlobalParam(misForward[l][w][1], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireFwdY);
            // thisGlobalParamVector.updateGlobalParam(misForward[l][w][2], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireFwdZ);
            //
            // thisGlobalParamVector.updateGlobalParam(misBackward[l][w][0], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireBwdX);
            // thisGlobalParamVector.updateGlobalParam(misBackward[l][w][1], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireBwdY);
            // thisGlobalParamVector.updateGlobalParam(misBackward[l][w][2], CDCAlignment::getGlobalUniqueID(), wire, CDCAlignment::wireBwdZ);

          }
        }
      }
    }
//    }

    void VXDGlobalParamInterface::setupAlignmentHierarchy(GlobalDerivativesHierarchy& hierarchy)
    {
      try {
        auto& rigidBodyHierarchy = dynamic_cast<RigidBodyHierarchy&>(hierarchy);
        auto& geo = VXD::GeoCache::getInstance();
        // Set-up hierarchy
        DBObjPtr<VXDAlignment> vxdAlignments;
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
      } catch (const std::bad_cast& e) {
        // Bad cast
      }
    }

  }
}