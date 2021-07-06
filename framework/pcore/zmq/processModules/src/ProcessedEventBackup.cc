/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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