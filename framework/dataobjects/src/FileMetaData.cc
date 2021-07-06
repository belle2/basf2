/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <boost/python/class.hpp>
#include <boost/python/copy_const_reference.hpp>

#include <framework/dataobjects/FileMetaData.h>
#include <framework/utilities/HTML.h>
#include <framework/utilities/KeyValuePrinter.h>

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;
using namespace boost::python;

FileMetaData::FileMetaData() :
  m_lfn(""), m_nEvents(0), m_experimentLow(0), m_runLow(0), m_eventLow(0),
  m_experimentHigh(0), m_runHigh(0), m_eventHigh(0), m_date(""), m_site(""), m_user(""), m_release(""),
  m_steering(""), m_isMC(true), m_mcEvents(0)
{
}

bool FileMetaData::containsEvent(int experiment, int run, unsigned int event) const
{
  if (experiment < m_experimentLow) return false;
  if (experiment == m_experimentLow) {
    if (run < m_runLow) return false;
    if ((run == m_runLow) && (event < m_eventLow)) return false;
  }
  if (experiment > m_experimentHigh) return false;
  if (experiment == m_experimentHigh) {
    if (run > m_runHigh) return false;
    if ((run == m_runHigh) && (event > m_eventHigh)) return false;
  }
  return true;
}

void FileMetaData::exposePythonAPI()
{
  //Note: these can only be used with update_file_metadata(), PyROOT is the more common interface
  class_<FileMetaData>("FileMetaData")
  .def("get_lfn", &FileMetaData::getLfn, return_value_policy<copy_const_reference>())
  .def("get_nevents", &FileMetaData::getNEvents)
  .def("get_experiment_low", &FileMetaData::getExperimentLow)
  .def("get_run_low", &FileMetaData::getRunLow)
  .def("get_event_low", &FileMetaData::getEventLow)
  .def("get_experiment_high", &FileMetaData::getExperimentHigh)
  .def("get_run_high", &FileMetaData::getRunHigh)
  .def("get_event_high", &FileMetaData::getEventHigh)
  .def("get_n_parents", &FileMetaData::getNParents)
  .def("get_parent", &FileMetaData::getParent, return_value_policy<copy_const_reference>())
  .def("get_date", &FileMetaData::getDate, return_value_policy<copy_const_reference>())
  .def("get_site", &FileMetaData::getSite, return_value_policy<copy_const_reference>())
  .def("get_user", &FileMetaData::getUser, return_value_policy<copy_const_reference>())
  .def("get_random_seed", &FileMetaData::getRandomSeed, return_value_policy<copy_const_reference>())
  .def("get_release", &FileMetaData::getRelease, return_value_policy<copy_const_reference>())
  .def("get_steering", &FileMetaData::getSteering, return_value_policy<copy_const_reference>())
  .def("is_mc", &FileMetaData::isMC)
  .def("get_mc_events", &FileMetaData::getMcEvents)
  .def("get_global_tag", &FileMetaData::getDatabaseGlobalTag, return_value_policy<copy_const_reference>())
  .def("get_data_description", &FileMetaData::getDataDescription, return_value_policy<copy_const_reference>())
  .def("set_lfn", &FileMetaData::setLfn);
}


void FileMetaData::Print(Option_t* option) const
{
  if (option && (option == std::string("steering"))) {
    cout << m_steering << endl;
    return;
  } else if (option && (option == std::string("json"))) {
    cout << getJsonStr() << endl;
    return;
  }
  const bool all = (option && option == std::string("all"));
  KeyValuePrinter printer(false);
  printer.put("LFN", m_lfn);
  printer.put("nEvents", m_nEvents);
  printer.put("range", std::to_string(m_experimentLow) + "/" + std::to_string(m_runLow) + "/" + std::to_string(m_eventLow)
              + " - "  + std::to_string(m_experimentHigh) + "/" + std::to_string(m_runHigh) + "/" + std::to_string(m_eventHigh));
  printer.put("parents", m_parentLfns);
  if (all) {
    printer.put("date", m_date);
    printer.put("site", m_site);
    printer.put("user", m_user);
    printer.put("randomSeed", m_randomSeed);
    printer.put("release", m_release);
    printer.put("isMC", m_isMC);
    printer.put("mcEvents", m_mcEvents);
    printer.put("globalTag", m_databaseGlobalTag);
    printer.put("dataDescription", m_dataDescription);
  }
  std::cout << "=== FileMetaData ===\n";
  std::cout << printer.string();
}

bool FileMetaData::read(std::istream& input, std::string& physicalFileName)
{
  physicalFileName = "";
  if (input.eof()) return false;

  std::string line;
  std::getline(input, line);
  boost::algorithm::trim(line);
  if (line.compare("<File>") != 0) return false;

  while (!input.eof()) {
    std::getline(input, line);
    boost::algorithm::trim(line);
    if (line.compare("</File>") == 0) return true;

    int pos = line.find('>') + 1;
    std::string tag = line.substr(0, pos);
    std::string value = line.substr(pos, line.rfind('<') - pos);
    if (tag.compare("<LFN>") == 0) {
      m_lfn = HTML::unescape(value);
    } else if (tag.compare("<PFN>") == 0) {
      physicalFileName = HTML::unescape(value);
    } else if (tag.compare("<ExperimentLow>") == 0) {
      m_experimentLow = stoi(value);
    } else if (tag.compare("<RunLow>") == 0) {
      m_runLow = stoi(value);
    } else if (tag.compare("<EventLow>") == 0) {
      m_eventLow = stoi(value);
    } else if (tag.compare("<ExperimentHigh>") == 0) {
      m_experimentHigh = stoi(value);
    } else if (tag.compare("<RunHigh>") == 0) {
      m_runHigh = stoi(value);
    } else if (tag.compare("<EventHigh>") == 0) {
      m_eventHigh = stoi(value);
    } else if (tag.compare("<Parents>") == 0) {
      pos = value.find(',');
      while (pos > 0) {
        m_parentLfns.push_back(HTML::unescape(value.substr(0, pos)));
        value.erase(0, pos + 1);
        pos = value.find(',');
      }
      m_parentLfns.push_back(HTML::unescape(value));
    }
  }

  return false;
}

bool FileMetaData::write(std::ostream& output, const std::string& physicalFileName) const
{
  output << "  <File>\n";
  output << "    <LFN>" << HTML::escape(m_lfn) << "</LFN>\n";
  if (!physicalFileName.empty()) {
    output << "    <PFN>" << HTML::escape(physicalFileName) << "</PFN>\n";
  }
  output << "    <ExperimentLow>" << m_experimentLow << "</ExperimentLow>\n";
  output << "    <RunLow>" << m_runLow << "</RunLow>\n";
  output << "    <EventLow>" << m_eventLow << "</EventLow>\n";
  output << "    <ExperimentHigh>" << m_experimentHigh << "</ExperimentHigh>\n";
  output << "    <RunHigh>" << m_runHigh << "</RunHigh>\n";
  output << "    <EventHigh>" << m_eventHigh << "</EventHigh>\n";
  if (!m_parentLfns.empty()) {
    output << "    <Parents>" << HTML::escape(m_parentLfns[0]);
    for (unsigned int parent = 1; parent < m_parentLfns.size(); parent++) {
      output << "," << HTML::escape(m_parentLfns[parent]);
    }
    output << "</Parents>\n";
  }
  output << "  </File>\n";

  return true;
}

std::string FileMetaData::getJsonStr() const
{
  nlohmann::json metadata = {
    {"LFN", m_lfn},
    {"nEvents", m_nEvents},
    {"experimentLow", m_experimentLow},
    {"runLow", m_runLow},
    {"eventLow", m_eventLow},
    {"experimentHigh", m_experimentHigh},
    {"runHigh", m_runHigh},
    {"eventHigh", m_eventHigh},
    {"parents", m_parentLfns},
    {"date", m_date},
    {"site", m_site},
    {"user", m_user},
    {"randomSeed", m_randomSeed},
    {"release", m_release},
    {"isMC", m_isMC},
    {"mcEvents", m_mcEvents},
    {"globalTag", m_databaseGlobalTag},
    {"dataDescription", m_dataDescription},
    {"steering", m_steering}
  };
  return metadata.dump(2);
}
