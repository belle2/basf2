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

#include <nlohmann/json.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;

DQMFileMetaData::DQMFileMetaData() :
  m_nEvents(0), m_experimentLow(0), m_runLow(0), m_eventLow(0),
  m_experimentHigh(0), m_runHigh(0), m_eventHigh(0), m_date(""), m_release(""),
  m_isMC(true), m_mcEvents(0)
{
}


void DQMFileMetaData::Print(Option_t* option) const
{

  const bool all = (option && option == std::string("all"));
  KeyValuePrinter printer(false);
  printer.put("nEvents", m_nEvents);
  printer.put("range", std::to_string(m_experimentLow) + "/" + std::to_string(m_runLow) + "/" + std::to_string(m_eventLow)
              + " - "  + std::to_string(m_experimentHigh) + "/" + std::to_string(m_runHigh) + "/" + std::to_string(m_eventHigh));

  if (all) {
    printer.put("date", m_date);
    printer.put("release", m_release);
    printer.put("isMC", m_isMC);
    printer.put("mcEvents", m_mcEvents);
    printer.put("globalTag", m_databaseGlobalTag);
  }
  std::cout << "=== DQMFileMetaData ===\n";
  std::cout << printer.string();
}

bool DQMFileMetaData::write(std::ostream& output, const std::string& physicalFileName) const
{
  output << "  <File>\n";
  output << "    <ExperimentLow>" << m_experimentLow << "</ExperimentLow>\n";
  output << "    <RunLow>" << m_runLow << "</RunLow>\n";
  output << "    <EventLow>" << m_eventLow << "</EventLow>\n";
  output << "    <ExperimentHigh>" << m_experimentHigh << "</ExperimentHigh>\n";
  output << "    <RunHigh>" << m_runHigh << "</RunHigh>\n";
  output << "    <EventHigh>" << m_eventHigh << "</EventHigh>\n";
  output << "  </File>\n";

  return true;
}

std::string DQMFileMetaData::getJsonStr() const
{
  nlohmann::json metadata = {
    {"nEvents", m_nEvents},
    {"experimentLow", m_experimentLow},
    {"runLow", m_runLow},
    {"eventLow", m_eventLow},
    {"experimentHigh", m_experimentHigh},
    {"runHigh", m_runHigh},
    {"eventHigh", m_eventHigh},
    {"date", m_date},
    {"release", m_release},
    {"isMC", m_isMC},
    {"mcEvents", m_mcEvents},
    {"globalTag", m_databaseGlobalTag},
  };
  return metadata.dump(2);
}
