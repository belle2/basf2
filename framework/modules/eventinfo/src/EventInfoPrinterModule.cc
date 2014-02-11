/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/modules/eventinfo/EventInfoPrinterModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
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
  StoreObjPtr<EventMetaData>::required();
}

void EventInfoPrinterModule::beginRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  B2INFO("========================================================================");
  B2INFO(">>> Start new run: " << eventMetaDataPtr->getRun());
  B2INFO("------------------------------------------------------------------------");
}


void EventInfoPrinterModule::event()
{
  //Print event meta data information
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (!eventMetaDataPtr) return;
  if (m_printTime) {
    using namespace chrono;
    long long int nsec = eventMetaDataPtr->getTime();
    high_resolution_clock::time_point time(duration_cast<high_resolution_clock::duration>(nanoseconds(nsec)));
    time_t ttime = high_resolution_clock::to_time_t(time);
    tm* tm = gmtime(&ttime);
    char timeStr[64];
    sprintf(timeStr, "%4d-%02d-%02d %02d:%02d:%02d.%06d", 1900 + tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, int((eventMetaDataPtr->getTime() / 1000) % 1000000));
    B2INFO(boost::format("EXP: %5d  RUN: %6d  EVT: %8d  TIME: %s") % eventMetaDataPtr->getExperiment() % eventMetaDataPtr->getRun() % eventMetaDataPtr->getEvent() % timeStr);
  } else {
    B2INFO(boost::format("EXP: %8d        RUN: %8d        EVT: %8d") % eventMetaDataPtr->getExperiment() % eventMetaDataPtr->getRun() % eventMetaDataPtr->getEvent());
  }
}


void EventInfoPrinterModule::endRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  B2INFO("------------------------------------------------------------------------");
  B2INFO("<<< End run: " << eventMetaDataPtr->getRun());
  B2INFO("========================================================================");
}
