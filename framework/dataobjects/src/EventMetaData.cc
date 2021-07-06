/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/EventMetaData.h>

#include <climits>

using namespace Belle2;

EventMetaData::EventMetaData(unsigned int event, int run, int experiment):
  m_event(event),
  m_run(run),
  m_subrun(0),
  m_experiment(experiment),
  m_production(0),
  m_time(0),
  m_parentLfn(""),
  m_generatedWeight(1.0),
  m_errorFlag(0)
{}

void EventMetaData::setEndOfData()
{
  m_event = UINT_MAX;
  m_run = INT_MAX;
  m_experiment = INT_MAX;
}

bool EventMetaData::isEndOfData() const
{
  return (m_event == UINT_MAX and m_run == INT_MAX and m_experiment == INT_MAX);
}

void EventMetaData::setEndOfRun(unsigned int experimentNumber, unsigned int runNumber)
{
  m_event = UINT_MAX;
  m_experiment = experimentNumber;
  m_run = runNumber;
}

bool EventMetaData::isEndOfRun() const
{
  return not isEndOfData() and (m_event == UINT_MAX);
}

bool EventMetaData::operator== (const EventMetaData& eventMetaData) const
{
  return ((m_event == eventMetaData.getEvent()) &&
          (m_run == eventMetaData.getRun()) &&
          (m_experiment == eventMetaData.getExperiment()));
}
