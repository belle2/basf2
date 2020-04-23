/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/TestTriggerTimeOffset/TestTriggerTimeOffsetModule.h>

#include <framework/core/Environment.h>
#include <framework/utilities/Utils.h>
#include <framework/gearbox/Const.h>
#include <iostream>

#include <chrono>
#include <set>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TestTriggerTimeOffset)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TestTriggerTimeOffsetModule::TestTriggerTimeOffsetModule() : Module()
{
  //Set module properties
  setDescription(
    "Sets the event meta data information (exp, run, evt). You must use this "
    "module to tell basf2 about the number of events you want to generate, "
    "unless you have an input module that already does so. Note that all "
    "experiment/run combinations specified must be unique."
  );

  //Parameter definition

  //  addParam("timerName", m_timerName,
  //     "name of timer store object", string(""));
}

TestTriggerTimeOffsetModule::~TestTriggerTimeOffsetModule() = default;

void TestTriggerTimeOffsetModule::initialize()
{
  //Load the timer object from the data store
  m_timer.isRequired();

}


void TestTriggerTimeOffsetModule::event()
{
  std::cout << "get clock TOP " << m_timer->getClock(Const::EDetector::TOP, "sampling") << std::endl;
  std::cout << "get clock SVD " << m_timer->getClock(Const::EDetector::SVD, "sampling") << std::endl;
  //  std::cout << "phase top " << m_timer->getPhase(Const::EDetector::SVD) << std::endl;
  std::cout << "tiggr bit position TOP " << m_timer->getTriggerBit(Const::EDetector::TOP, "sampling") << " [ns] " << std::endl;
  std::cout << "tiggr bit position SVD " << m_timer->getTriggerBit(Const::EDetector::SVD, "sampling") << " [ns] " << std::endl;
  std::cout << "tiggr bit position ECL sampling " << m_timer->getTriggerBit(Const::EDetector::ECL,
            "sampling") << " [ns] " << std::endl;
  std::cout << "tiggr bit position ECL fitting " << m_timer->getTriggerBit(Const::EDetector::ECL, "fitting") << " [ns] " << std::endl;
  //std::cout << "tiggr bit position CDC fitting " << m_timer->getTriggerBit(Const::EDetector::CDC,"sampling") << " [ns] " << std::endl;

}
