/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/AlignmentGenerator/AlignmentGeneratorModule.h>

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
REG_MODULE(AlignmentGenerator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AlignmentGeneratorModule::AlignmentGeneratorModule() : Module()
{
  std::vector<std::string> emptyData;

  setPropertyFlags(c_ParallelProcessingCertified);
  // Set module properties
  setDescription("Generate VXD alignment and store in database");

  // Parameter definitions
  addParam("experimentLow", m_experimentLow, "Min experiment number to generate this alignment for", 0);
  addParam("runLow", m_runLow, "Min run number to generate this alignment for", 0);
  addParam("experimentHigh", m_experimentHigh, "Max experiment number to generate this alignment for", -1);
  addParam("runHigh", m_runHigh, "Max run number to generate this alignment for", -1);
  addParam("data", m_data,
           "Data for alignment in format ['layer.ladder.sensor, parameter_no, distribution=fix|gaus|uniform, value', ...]",
           emptyData);
  addParam("name", m_name, "Name of generated alignment in database", std::string(""));

}

void AlignmentGeneratorModule::initialize()
{
  IntervalOfValidity iov(m_experimentLow, m_runLow, m_experimentHigh, m_runHigh);
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

  if (m_name == "")
    Database::Instance().storeData(data, iov);
  else
    Database::Instance().storeData(m_name, data, iov);

}


