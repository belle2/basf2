/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/class.hpp>

#include <framework/dataobjects/FileMetaData.h>
#include <framework/utilities/HTML.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>


using namespace std;
using namespace Belle2;
using namespace boost::python;

FileMetaData::FileMetaData() :
  m_lfn(""), m_nEvents(0), m_experimentLow(0), m_runLow(0), m_eventLow(0),
  m_experimentHigh(0), m_runHigh(0), m_eventHigh(0), m_date(""), m_site(""), m_user(""), m_release(""),
  m_steering(""), m_mcEvents(0)
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
  class_<FileMetaData>("FileMetaData")
  .def("get_lfn", &FileMetaData::getLfn)
  .def("get_nevents", &FileMetaData::getNEvents)
  .def("get_experiment_low", &FileMetaData::getExperimentLow)
  .def("get_run_low", &FileMetaData::getRunLow)
  .def("get_event_low", &FileMetaData::getEventLow)
  .def("get_experiment_high", &FileMetaData::getExperimentHigh)
  .def("get_run_high", &FileMetaData::getRunHigh)
  .def("get_event_high", &FileMetaData::getEventHigh)
  .def("get_n_parents", &FileMetaData::getNParents)
  .def("get_parent", &FileMetaData::getParent)
  .def("get_date", &FileMetaData::getDate)
  .def("get_site", &FileMetaData::getSite)
  .def("get_user", &FileMetaData::getUser)
  .def("get_random_seed", &FileMetaData::getRandomSeed)
  .def("get_release", &FileMetaData::getRelease)
  .def("get_steering", &FileMetaData::getSteering)
  .def("get_mc_events", &FileMetaData::getMcEvents)
  .def("get_global_tag", &FileMetaData::getDatabaseGlobalTag)
  .def("set_lfn", &FileMetaData::setLfn);
}

void FileMetaData::Print(Option_t* option) const
{
  if (option && (strcmp(option, "steering") == 0)) {
    cout << m_steering << endl;
    return;
  }

  const bool all = (option && (strcmp(option, "all") == 0));
  cout << "FileMetaData" << endl;
  cout << "  LFN    : " << m_lfn << endl;
  cout << "  #event : " << m_nEvents << endl;
  cout << "  range  : " << m_experimentLow << "/" << m_runLow << "/" << m_eventLow
       << " - "  << m_experimentHigh << "/" << m_runHigh << "/" << m_eventHigh << endl;
  cout << "  parents:";
  for (std::string parent : m_parentLfns)
    cout << " " << parent;
  cout << endl;
  if (all) {
    cout << "  date   : " << m_date << endl;
    cout << "  site   : " << m_site << endl;
    cout << "  user   : " << m_user << endl;
    cout << "  seed   : " << m_randomSeed << endl;
    cout << "  release: " << m_release << endl;
    cout << "  #MC    : " << m_mcEvents << endl;
    cout << "globalTag: " << m_databaseGlobalTag << endl;
  }
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

    int pos = line.find(">") + 1;
    std::string tag = line.substr(0, pos);
    std::string value = line.substr(pos, line.rfind("<") - pos);
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
      pos = value.find(",");
      while (pos > 0) {
        m_parentLfns.push_back(HTML::unescape(value.substr(0, pos)));
        value.erase(0, pos + 1);
        pos = value.find(",");
      }
      m_parentLfns.push_back(HTML::unescape(value));
    }
  }

  return false;
}

bool FileMetaData::write(std::ostream& output, std::string physicalFileName)
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
