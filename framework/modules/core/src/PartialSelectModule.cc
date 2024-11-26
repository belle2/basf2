/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/modules/core/PartialSelectModule.h>
#include <framework/core/Environment.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PartialSelect);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PartialSelectModule::PartialSelectModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(
    This module helps you set the interval of events to process. It
    returns True inside the set window and False outside. 

    The input parameters are fractions which can be interpretted as the
    fraction of leading and (1-)fraction of trailing events that will be
    skipped. Internally, the total number of events of the input file is 
    used to determine the event selection window boundaries. 

    basf2 conditional paths can then be used to select events that pass 
    this module for further processing.

    This module can be used with GRID jobs when dataset collections are 
    passed as input. It helps you control the range of events per file so
    that you can avoid processing all of the events but still cover all
    the files in the collection. 

    NOTE: This module has to be added directly after the input module so 
    as to filter out events before any other processing. It also only 
    works with one input file for now.
  )DOC");

  // Parameter definitions
  addParam(
    "entryStart",
    m_entryStart,
    "The product of entryStart fraction and total number of events in each "
    "file will determine the first event to begin processing with. Module will"
    " return False for all leading events before this. By default(0.0) module "
    "will start with the first event.",
    0.);
  addParam(
    "entryStop",
    m_entryStop,
    "The product of entryStop fraction and total number of events in each file"
    " will determine the last event to end processing with. Module will return"
    " False for all trailing events after this. By default(1.0) module will "
    "end wtih the last event.",
    1.);
}

void PartialSelectModule::initialize()
{
  // Have we received meaningful values?
  const bool isOutOfBounds =
    m_entryStart < 0 || m_entryStop < 0 ||
    m_entryStart > 1 || m_entryStop > 1 ||
    m_entryStart > m_entryStop;

  if (isOutOfBounds)
    B2FATAL("Either the entryStart and entryStop passed are beyond the accepted"
            " range or they have been swapped: please refer to the 'entryStart'"
            " and 'entryStop' descriptions of the PartialSelect module.");
  m_nTotal = Environment::Instance().getNumberOfEvents();
  B2INFO("Total number of events :"
         << m_nTotal << ".");
}

void PartialSelectModule::event()
{
  m_events += 1;
  // Are we inside the passed interval?
  if (m_events > m_entryStart * m_nTotal and m_events <= m_entryStop * m_nTotal) {
    m_returnValue = true;
  }
  // If not, we skip the events.
  else {
    m_returnValue = false;
  }
  // Mark the boundary events in log.
  if (m_events > m_entryStart * m_nTotal and m_events <= m_entryStart * m_nTotal + 1) {
    B2INFO("First event passing the PartialSelect range: "
           << m_events << ".");
  }
  if (m_events <= m_entryStop * m_nTotal and m_events > m_entryStop * m_nTotal - 1) {
    B2INFO("Last event passing the PartialSelect range: "
           << m_events << ".");
  }
  setReturnValue(m_returnValue);
}
