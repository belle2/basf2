/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/core/EventInfoPrinterModule.h>

#include <boost/format.hpp>
#include <chrono>
#include <ctime>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventInfoPrinter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventInfoPrinterModule::EventInfoPrinterModule() : Module()
{
  //Set module properties
  setDescription("Prints the current event meta data information (exp, run, event numbers).");

  addParam("printTime", m_printTime, "Print time in addition to exp/run/evt numbers.", false);
}


EventInfoPrinterModule::~EventInfoPrinterModule()
{

}

void EventInfoPrinterModule::initialize()
{
  m_eventMetaData.isRequired();
}

void EventInfoPrinterModule::beginRun()
{
  B2INFO("========================================================================");
  B2INFO(">>> Start new run: " << m_eventMetaData->getRun());
  B2INFO("------------------------------------------------------------------------");
}


void EventInfoPrinterModule::event()
{
  //Print event meta data information
  if (!m_eventMetaData) return;
  if (m_printTime) {
    B2INFO(boost::format("EXP: %5d  RUN: %6d  EVT: %8d  TIME: %s") % m_eventMetaData->getExperiment() % m_eventMetaData->getRun() %
           m_eventMetaData->getEvent() % formatDateTime(m_eventMetaData->getTime()));
  } else {
    B2INFO(boost::format("EXP: %8d        RUN: %8d        EVT: %8d") % m_eventMetaData->getExperiment() % m_eventMetaData->getRun() %
           m_eventMetaData->getEvent());
  }
}


void EventInfoPrinterModule::endRun()
{
  B2INFO("------------------------------------------------------------------------");
  B2INFO("<<< End run: " << m_eventMetaData->getRun());
  B2INFO("========================================================================");
}

std::string EventInfoPrinterModule::formatDateTime(long long int nsec)
{
  chrono::high_resolution_clock::time_point time(chrono::duration_cast<chrono::high_resolution_clock::duration>(
                                                   chrono::nanoseconds(nsec)));
  time_t ttime = chrono::high_resolution_clock::to_time_t(time);
  tm* tm = gmtime(&ttime);
  char timeStr[32];
  strftime(timeStr, 32, "%F %T", tm);
  unsigned int sec_decimals = (nsec / 1000) % 1000000;

  return str(boost::format("%s.%06d") % timeStr % sec_decimals);
}
