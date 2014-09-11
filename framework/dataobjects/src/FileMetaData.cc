/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/class.hpp>
#include <boost/algorithm/string.hpp>

#include <framework/dataobjects/FileMetaData.h>
#include <framework/logging/Logger.h>

#include <iostream>


using namespace std;
using namespace Belle2;
using namespace boost::python;

ClassImp(FileMetaData)

FileMetaData::FileMetaData() :
  m_id(0), m_guid(""), m_lfn(""), m_events(0), m_experimentLow(0), m_runLow(0), m_eventLow(0),
  m_experimentHigh(0), m_runHigh(0), m_eventHigh(0), m_release(""), m_timeStamp(0), m_site(""), m_user(""), m_logFile(0),
  m_randomSeed(0), m_random(0), m_steering("")
{
}

bool FileMetaData::containsEvent(unsigned long experiment, unsigned long run, unsigned long event) const
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
  .def("get_id", &FileMetaData::getId)
  .def("get_guid", &FileMetaData::getGuid)
  .def("get_lfn", &FileMetaData::getLfn)
  .def("get_events", &FileMetaData::getEvents)
  .def("get_experiment_low", &FileMetaData::getExperimentLow)
  .def("get_run_low", &FileMetaData::getRunLow)
  .def("get_event_low", &FileMetaData::getEventLow)
  .def("get_experiment_high", &FileMetaData::getExperimentHigh)
  .def("get_run_high", &FileMetaData::getRunHigh)
  .def("get_event_high", &FileMetaData::getEventHigh)
  .def("get_n_parents", &FileMetaData::getNParents)
  .def("get_parent", &FileMetaData::getParent)
  .def("get_release", &FileMetaData::getRelease)
  .def("get_timestamp", &FileMetaData::getTimeStamp)
  .def("get_site", &FileMetaData::getSite)
  .def("get_user", &FileMetaData::getUser)
  .def("get_logfile", &FileMetaData::getLogFile)
  .def("get_random_seed", &FileMetaData::getRandomSeed)
  .def("get_random", &FileMetaData::getRandom)
  .def("get_steering", &FileMetaData::getSteering)
  .def("set_ids", &FileMetaData::setIds);
}

void FileMetaData::Print(Option_t* option) const
{
  if (option && (strcmp(option, "steering") == 0)) {
    cout << m_steering << endl;
    return;
  }

  bool all = (option && (strcmp(option, "all") == 0));
  cout << "FileMetaData" << endl;
  cout << "  ID     : " << m_id << endl;
  cout << "  LFN    : " << m_lfn << endl;
  if (all)
    cout << "  GUID   : " << m_guid << endl;
  cout << "  #event : " << m_events << endl;
  cout << "  range  : " << m_experimentLow << "/" << m_runLow << "/" << m_eventLow << " - "  << m_experimentHigh << "/" << m_runHigh << "/" << m_eventHigh << endl;
  cout << "  parents:";
  for (int parent : m_parents) cout << " " << parent;
  cout << endl;
  if (all) {
    cout << "  release: " << m_release << endl;
    cout << "  time   : " << m_timeStamp << endl;
    cout << "  site   : " << m_site << endl;
    cout << "  user   : " << m_user << endl;
    cout << "  log    : " << m_logFile << endl;
    cout << "  seed   : " << m_randomSeed << endl;
    cout << "  random : " << m_random.GetSeed() << endl;
  }
}

namespace Belle2 {

  std::istream& operator>> (std::istream& input, FileMetaData& metaData)
  {
    new(&metaData) FileMetaData;
    if (input.eof()) return input;

    std::string line;
    std::getline(input, line);
    boost::algorithm::trim(line);
    if (line.compare("<File>") != 0) return input;

    while (!input.eof()) {
      std::getline(input, line);
      boost::algorithm::trim(line);
      if (line.compare("</File>") == 0) return input;

      int pos = line.find(">") + 1;
      std::string tag = line.substr(0, pos);
      std::string value = line.substr(pos, line.rfind("<") - pos);
      if (tag.compare("<ID>") == 0) {
        metaData.m_id = atoi(value.c_str());
      } else if (tag.compare("<LFN>") == 0) {
        metaData.m_lfn = value;
      } else if (tag.compare("<ExperimentLow>") == 0) {
        metaData.m_experimentLow = atoi(value.c_str());
      } else if (tag.compare("<RunLow>") == 0) {
        metaData.m_runLow = atoi(value.c_str());
      } else if (tag.compare("<EventLow>") == 0) {
        metaData.m_eventLow = atoi(value.c_str());
      } else if (tag.compare("<ExperimentHigh>") == 0) {
        metaData.m_experimentHigh = atoi(value.c_str());
      } else if (tag.compare("<RunHigh>") == 0) {
        metaData.m_runHigh = atoi(value.c_str());
      } else if (tag.compare("<EventHigh>") == 0) {
        metaData.m_eventHigh = atoi(value.c_str());
      } else if (tag.compare("<Parents>") == 0) {
        pos = value.find(",");
        while (pos > 0) {
          metaData.m_parents.push_back(atoi(value.substr(0, pos).c_str()));
          value.erase(0, pos + 1);
          pos = value.find(",");
        }
        metaData.m_parents.push_back(atoi(value.c_str()));
      }
    }

    return input;
  }

  std::ostream& operator<< (std::ostream& output, const FileMetaData& metaData)
  {
    output << "  <File>\n";
    output << "    <ID>" << metaData.m_id << "</ID>\n";
    output << "    <LFN>" << metaData.m_lfn << "</LFN>\n";
    output << "    <ExperimentLow>" << metaData.m_experimentLow << "</ExperimentLow>\n";
    output << "    <RunLow>" << metaData.m_runLow << "</RunLow>\n";
    output << "    <EventLow>" << metaData.m_eventLow << "</EventLow>\n";
    output << "    <ExperimentHigh>" << metaData.m_experimentHigh << "</ExperimentHigh>\n";
    output << "    <RunHigh>" << metaData.m_runHigh << "</RunHigh>\n";
    output << "    <EventHigh>" << metaData.m_eventHigh << "</EventHigh>\n";
    if (!metaData.m_parents.empty()) {
      output << "    <Parents>" << metaData.m_parents[0];
      for (unsigned int parent = 1; parent < metaData.m_parents.size(); parent++) {
        output << "," << metaData.m_parents[parent];
      }
      output << "</Parents>\n";
    }
    output << "  </File>\n";

    return output;
  }

}
