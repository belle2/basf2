/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/processModules/ProcessedEventBackup.h>

using namespace Belle2;

ProcessedEventBackup::ProcessedEventBackup(std::unique_ptr<EvtMessage> evtMsg,
                                           const EventMetaData& evtMetaData,
                                           unsigned int workerId)
  : m_eventMessage(std::move(evtMsg)), m_eventMetaData(evtMetaData), m_workerId(workerId)
{
}

const EventMetaData& ProcessedEventBackup::getEventMetaData() const
{
  return m_eventMetaData;
}

const ProcessedEventBackup::TimeStamp& ProcessedEventBackup::getTimestamp() const
{
  return m_timestamp;
}

const unsigned int& ProcessedEventBackup::getWorkerId() const
{
  return m_workerId;
}

bool ProcessedEventBackup::operator==(const EventMetaData& rhs)
{
  return m_eventMetaData == rhs;
}