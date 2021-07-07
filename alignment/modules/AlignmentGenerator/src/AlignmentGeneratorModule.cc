/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/modules/AlignmentGenerator/AlignmentGeneratorModule.h>

#include <alignment/dbobjects/VXDAlignment.h>
#include <framework/database/Database.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <TRandom.h>

#include <boost/algorithm/string.hpp>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AlignmentGenerator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AlignmentGeneratorModule::AlignmentGeneratorModule() : Module()
{
  std::vector<std::string> emptyData;
  std::vector<int> infiniteIov{0, 0, -1, -1};

  setPropertyFlags(c_ParallelProcessingCertified);
  // Set module properties
  setDescription("Generates VXD alignment."
                 "Generated alignment overrides any existing one in reconstruction if done with this module. "
                 "The generated object can also be stored as payload in the (local) DB, to keep track of it."
                );

  // Parameter definitions
  addParam("payloadIov", m_payloadIov, "IoV of the payload to be created. List "
           "of four numbers: first experiment, first run, last experiment, "
           "last run", infiniteIov);
  addParam("createPayload", m_createPayload, "Store the generated alignment as payload in DB?", bool(false));
  addParam("data", m_data,
           "Data for alignment in format ['layer.ladder.sensor, parameter_no, distribution=fix|gaus|uniform, value', ...]",
           emptyData);
  addParam("payloadName", m_payloadName, "Name of generated alignment payload in database. If empty, default is used",
           std::string(""));

}

void AlignmentGeneratorModule::initialize()
{
  auto data = new VXDAlignment();

  for (auto& id : VXD::GeoCache::getInstance().getListOfSensors()) {
    for (auto paramData : m_data) {
      std::vector<std::string> paramDataParts;
      boost::algorithm::split(paramDataParts, paramData, boost::is_any_of(","));
      if (paramDataParts.size() != 4)
        B2FATAL("Error parsing alignment data.");

      boost::trim(paramDataParts[0]);
      boost::trim(paramDataParts[1]);
      boost::trim(paramDataParts[2]);
      boost::trim(paramDataParts[3]);

      VxdID idMask(paramDataParts[0]);
      int paramID = std::stoi(paramDataParts[1]);
      std::string distro = paramDataParts[2];
      double value = std::stod(paramDataParts[3]);

      double generatedValue = 0.;
      if (distro == "fix") generatedValue = value;
      else if (distro == "gaus") generatedValue = gRandom->Gaus(0., value);
      else if (distro == "uniform") generatedValue = gRandom->Uniform(-value, value);
      else B2FATAL("Unknown distribution for parameter generation: " << distro << " Valid options are fix|gaus|uniform");

      if (idMask.getLayerNumber() && id.getLayerNumber() != idMask.getLayerNumber()) continue;
      if (idMask.getLadderNumber() && id.getLadderNumber() != idMask.getLadderNumber()) continue;
      if (idMask.getSensorNumber() && id.getSensorNumber() != idMask.getSensorNumber()) continue;

      data->set(id, paramID, generatedValue);
    }
  }

  std::string name = (m_payloadName == "") ? DBStore::objectName<VXDAlignment>("") : m_payloadName;

  B2WARNING("Overriding VXDAlignment in DBStore with new object. This will affect reconstruction if done in this job.");
  DBStore::Instance().addConstantOverride(name, new VXDAlignment(*data));

  if (m_createPayload) {
    if (m_payloadIov.size() != 4)
      B2FATAL("Payload IoV incorrect. Should be list of four numbers.");

    B2INFO("Storing VXDAlignment payload in DB.");
    IntervalOfValidity iov(m_payloadIov[0], m_payloadIov[1], m_payloadIov[2], m_payloadIov[3]);
    Database::Instance().storeData(name, data, iov);
  }

}


