/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/mixbkg/EventRandomizerModule.h>
#include <framework/core/InputController.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventRandomizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventRandomizerModule::EventRandomizerModule() : Module(), m_selector(0)
{
  //Set module properties
  setDescription("Randomize the sequence of events read by RootInput.");
  // This module talks to InputController, so it is not parallelizable.
  // No parameters.
}

void EventRandomizerModule::initialize()
{
  // No StoreArray or RelationsArray registration.

  // Number of events on input
  int nBaseEvents = InputController::numEntries();

  // Initialize the event randomizer
  m_selector = new RandomPermutation(nBaseEvents);
  // Set the first event number to read in InputController
  // InputController::setNextEntry(m_selector->getNext());

  B2INFO("=======================================================")
  B2INFO("                  EventRandomizer                      ")
  B2INFO("-------------------------------------------------------")
  B2INFO("Events on input      : " << nBaseEvents)
  B2INFO("Events will be processed in random order.")
  B2INFO("-------------------------------------------------------")
}


void EventRandomizerModule::event()
{
  // If all input events have been processed, do nothing.
  if (m_selector->isFinished()) {
    setReturnValue(1); // Signal to terminate
  } else {
    setReturnValue(0);
    InputController::setNextEntry(m_selector->getNext());
  }
}


void EventRandomizerModule::terminate()
{
  delete m_selector;
}

