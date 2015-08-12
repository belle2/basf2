/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/PedeSteeringCreator/PedeSteeringCreatorModule.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <boost/algorithm/string.hpp>
#include <alignment/dataobjects/PedeSteering.h>
#include <framework/datastore/StoreObjPtr.h>
#include <alignment/GlobalLabel.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PedeSteeringCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PedeSteeringCreatorModule::PedeSteeringCreatorModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  std::vector<std::string> commands;
  commands.push_back("method inversion 3 0.1");
  std::vector<std::string> parameters;
  // Set module properties
  setDescription("Create PedeSteering to configure Millepede");

  // Parameter definitions
  addParam("name", m_name, "Name of the persistent PedeSteering StoreObjPtr to be created", std::string("PedeSteering"));
  addParam("commands", m_commands, "List of plain Pede commands", commands);
  addParam("parameters", m_parameters, "List of 'layer.ladder.sensor.param:value,presigma' (ints) to be set/fixed(presigma<0)",
           parameters);

}

void PedeSteeringCreatorModule::initialize()
{
  StoreObjPtr<PedeSteering> steer(m_name, DataStore::c_Persistent);
  steer.registerInDataStore(m_name, DataStore::c_Persistent);
  steer.construct(steer.getName() + ".txt");

  for (auto command : m_commands)
    steer->command(command);

  vector<int> labels;
  vector<double> values;
  vector<double> presigmas;

  for (auto& id : VXD::GeoCache::getInstance().getListOfSensors()) {
    for (auto paramData : m_parameters) {
      std::vector<std::string> paramDataParts;
      boost::algorithm::split(paramDataParts, paramData, boost::is_any_of(":"));
      if (paramDataParts.size() != 2)
        B2FATAL("Error parsing parameter data.");

      boost::trim(paramDataParts[0]);
      boost::trim(paramDataParts[1]);

      std::vector<std::string> paramIdParts;
      boost::algorithm::split(paramIdParts, paramDataParts[0], boost::is_any_of("."));

      if (paramIdParts.size() != 4)
        B2FATAL("Error parsing parameter id data.");

      std::vector<std::string> value_presigma;
      boost::algorithm::split(value_presigma, paramDataParts[1], boost::is_any_of(","));

      if (value_presigma.size() != 2)
        B2FATAL("Error parsing value and presigma data.");

      VxdID idMask(std::stoi(paramIdParts[0]), std::stoi(paramIdParts[1]), std::stoi(paramIdParts[2]));
      int paramID = std::stoi(paramIdParts[3]);
      double value = std::stod(value_presigma[0]);
      double presigma = std::stod(value_presigma[1]);


      if (idMask.getLayerNumber() && id.getLayerNumber() != idMask.getLayerNumber()) continue;
      if (idMask.getLadderNumber() && id.getLadderNumber() != idMask.getLadderNumber()) continue;
      if (idMask.getSensorNumber() && id.getSensorNumber() != idMask.getSensorNumber()) continue;

      if (paramID == 0) {
        for (int ipar = 1; ipar <= 6; ipar++) {
          GlobalLabel label(id, ipar);
          labels.push_back(label.label());
          values.push_back(value);
          presigmas.push_back(presigma);
        }
      } else {
        GlobalLabel label(id, paramID);
        labels.push_back(label.label());
        values.push_back(value);
        presigmas.push_back(presigma);
      }
    }
  }
  steer->fixParameters(labels, values, presigmas);
  steer->make();
}

void PedeSteeringCreatorModule::beginRun()
{
}


