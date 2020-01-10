/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/dataobjects/DQMFileMetaData.h>


#include <framework/utilities/HTML.h>
#include <framework/utilities/KeyValuePrinter.h>

//#include <nlohmann/json.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;

DQMFileMetaData::DQMFileMetaData() :
  m_nEvents(0), m_experiment(0), m_run(0), m_date(""), m_release(""), m_databaseGlobalTag(""), m_procID(""),
  m_isMC(true), m_mcEvents(0)
{
}


void DQMFileMetaData::Print(Option_t* option) const
{

  const bool all = (option && option == std::string("all"));
  KeyValuePrinter printer(false);
  printer.put("nEvents", m_nEvents);
  printer.put("Experiment", m_experiment);
  printer.put("Run", m_run);

  if (all) {
    printer.put("run date", m_date);
    printer.put("release", m_release);
    printer.put("isMC", m_isMC);
    printer.put("mcEvents", m_mcEvents);
    printer.put("globalTag", m_databaseGlobalTag);
  }
  std::cout << "=== DQMFileMetaData ===\n";
  std::cout << printer.string();
}


std::string DQMFileMetaData::getJsonStr() const
{
  /*nlohmann::json metadata = {
    {"nEvents", m_nEvents},
    {"experiment", m_experiment},
    {"run", m_run},
    {"run date", m_date},
    {"release", m_release},
    {"isMC", m_isMC},
    {"mcEvents", m_mcEvents},
    {"globalTag", m_databaseGlobalTag},
  };
  return metadata.dump(2);*/
}
