/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MisalignmentGenerator/MisalignmentGeneratorModule.h>

#include <alignment/dbobjects/VXDAlignment.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/database/Database.h>

#include <boost/algorithm/string.hpp>

#include <vxd/geometry/GeoCache.h>
#include <TRandom.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MisalignmentGenerator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MisalignmentGeneratorModule::MisalignmentGeneratorModule() : Module()
{
  std::vector<std::string> emptyData;

  setPropertyFlags(c_ParallelProcessingCertified);
  // Set module properties
  setDescription("Generate VXD misalignment and store in database");

  // Parameter definitions
  addParam("experimentLow", m_experimentLow, "Min experiment number to generate this misalignment for", 0);
  addParam("runLow", m_runLow, "Min run number to generate this misalignment for", 0);
  addParam("experimentHigh", m_experimentHigh, "Max experiment number to generate this misalignment for", -1);
  addParam("runHigh", m_runHigh, "Max run number to generate this misalignment for", -1);
  addParam("data", m_data, "Data for misalignment generation in format ['layer.ladder.sensor,parameter_no,distribution,value', ...]",
           emptyData);
  addParam("name", m_name, "Name of generated misalignment in database", std::string("VXDMisalignment"));

}

void MisalignmentGeneratorModule::initialize()
{
  IntervalOfValidity iov(m_experimentLow, m_runLow, m_experimentHigh, m_runHigh);
  auto data = new VXDAlignment();

  for (auto& id : VXD::GeoCache::getInstance().getListOfSensors()) {
    for (auto paramData : m_data) {
      std::vector<std::string> paramDataParts;
      boost::algorithm::split(paramDataParts, paramData, boost::is_any_of(","));
      if (paramDataParts.size() != 4)
        B2FATAL("Error parsing misalignment data.");

      boost::trim(paramDataParts[0]);
      boost::trim(paramDataParts[1]);
      boost::trim(paramDataParts[2]);
      boost::trim(paramDataParts[3]);

      VxdID idMask(paramDataParts[0]);
      int paramID = std::stoi(paramDataParts[1]);
      std::string distro = paramDataParts[2];
      double value = std::stod(paramDataParts[3]);

      if (idMask.getLayerNumber() && id.getLayerNumber() != idMask.getLayerNumber()) continue;
      if (idMask.getLadderNumber() && id.getLadderNumber() != idMask.getLadderNumber()) continue;
      if (idMask.getSensorNumber() && id.getSensorNumber() != idMask.getSensorNumber()) continue;

      if (distro == "gaus") value = gRandom->Gaus(0., value);

      data->set(id, paramID, value);
    }
  }

  data->dump();

  Database::Instance().storeData(m_name, data, iov);
}


