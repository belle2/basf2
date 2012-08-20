/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>

#include <framework/dataobjects/FileMetaData.h>


using namespace std;
using namespace Belle2;
using namespace boost::python;

ClassImp(FileMetaData)

FileMetaData::FileMetaData() :
  m_id(0), m_guid(""), m_lfn(""), m_events(0), m_experiment(0), m_runLow(0), m_eventLow(0),
  m_runHigh(0), m_eventHigh(0), m_release(""), m_timeStamp(0), m_site(""), m_user(""), m_logFile(0),
  m_randomSeed(0), m_random(0), m_steering("")
{
}

void FileMetaData::exposePythonAPI()
{
  class_<FileMetaData>("FileMetaData")
  .def("get_id", &FileMetaData::getId)
  .def("get_guid", &FileMetaData::getGuid)
  .def("get_lfn", &FileMetaData::getLfn)
  .def("get_events", &FileMetaData::getEvents)
  .def("get_experiment", &FileMetaData::getExperiment)
  .def("get_run_low", &FileMetaData::getRunLow)
  .def("get_event_low", &FileMetaData::getEventLow)
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
