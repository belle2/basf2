/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
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
    "Simple module to test the trigger offset object used in simulation."
    "First import object from datastore then ask value of trigger offset"
    "for different sub detectors."
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
  std::cout << "get revo9 " << m_timer->getTriggerBitWrtRevo9() << std::endl;
  std::cout << "get clock TOP " << m_timer->getClock(Const::EDetector::TOP, "sampling") << std::endl;
  std::cout << "get clock SVD " << m_timer->getClock(Const::EDetector::SVD, "sampling") << std::endl;
  std::cout << "tigger bit position TOP " << m_timer->getTriggerOffset(Const::EDetector::TOP, "sampling") << " [ns] " << std::endl;
  std::cout << "tigger bit position SVD " << m_timer->getTriggerOffset(Const::EDetector::SVD, "sampling") << " [ns] " << std::endl;
  std::cout << "tigger bit position ECL sampling " << m_timer->getTriggerOffset(Const::EDetector::ECL,
            "sampling") << " [ns] " << std::endl;
  std::cout << "tigger bit position ECL fitting " << m_timer->getTriggerOffset(Const::EDetector::ECL,
            "fitting") << " [ns] " << std::endl;

}
