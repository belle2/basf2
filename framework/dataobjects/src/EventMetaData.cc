/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/EventMetaData.h>

#include <climits>

using namespace Belle2;

ClassImp(EventMetaData)

EventMetaData::EventMetaData(unsigned long event, unsigned long run, unsigned long experiment):
  m_event(event),
  m_run(run),
  m_subrun(0),
  m_experiment(experiment),
  m_time(0),
  m_parent_index(UINT_MAX),
  m_generated_weight(1.0)
{}

void EventMetaData::setEndOfData()
{
  m_event = UINT_MAX;
  m_run = UINT_MAX;
  m_experiment = UINT_MAX;
}

bool EventMetaData::isEndOfData() const
{
  return (m_event == UINT_MAX and m_run == UINT_MAX and m_experiment == UINT_MAX);
}

bool EventMetaData::operator== (const EventMetaData& eventMetaData) const
{
  return ((m_event == eventMetaData.getEvent()) &&
          (m_run == eventMetaData.getRun()) &&
          (m_experiment == eventMetaData.getExperiment()));
}
