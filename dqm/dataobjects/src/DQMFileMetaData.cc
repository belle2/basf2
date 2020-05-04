/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
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
  m_nEvents(0), m_experiment(0), m_run(0), m_date(""), m_release(""), m_procID(""), m_rtype(""), m_isMC(false),
  m_databaseGlobalTag("")
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
    printer.put("run type", m_rtype);
    printer.put("run date", m_date);
    printer.put("processing", m_procID);
    printer.put("release", m_release);
    printer.put("isMC", m_isMC);
    printer.put("globalTag", m_databaseGlobalTag);
  }
  std::cout << "=== DQMFileMetaData ===\n";
  std::cout << printer.string();
  std::cout << "=======================\n";
}
