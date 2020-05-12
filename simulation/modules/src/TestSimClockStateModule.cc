/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/TestSimClockStateModule.h>

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
REG_MODULE(TestSimClockState)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TestSimClockStateModule::TestSimClockStateModule() : Module()
{
  //Set module properties
  setDescription(
    "Simple module to test the trigger offset object used in simulation."
    "First import object from datastore then ask value of trigger offset"
    "for different sub detectors."
  );

  //Parameter definition

  //  addParam("timerName",m_clockStateName,
  //     "name of timer store object", string(""));
}

TestSimClockStateModule::~TestSimClockStateModule() = default;

void TestSimClockStateModule::initialize()
{
  //Load the timer object from the data store
  m_clockState.isRequired();

}


void TestSimClockStateModule::event()
{
  std::cout << "get revo9 status " << m_clockState->getRevo9Status() << std::endl;
  std::cout << "get clock frequency TOP " << m_clockState->getClockFreq(Const::EDetector::TOP, "sampling") << std::endl;
  std::cout << "get clock frequency SVD " << m_clockState->getClockFreq(Const::EDetector::SVD, "sampling") << std::endl;
  std::cout << "tigger signal offsetTOP " << m_clockState->getTriggerOffset(Const::EDetector::TOP,
            "sampling") << " [ns] " << std::endl;
  std::cout << "tigger signal offset SVD " << m_clockState->getTriggerOffset(Const::EDetector::SVD,
            "sampling") << " [ns] " << std::endl;
  std::cout << "tigger signal offset ECL sampling " << m_clockState->getTriggerOffset(Const::EDetector::ECL,
            "sampling") << " [ns] " << std::endl;
  std::cout << "tigger signal offset ECL fitting " << m_clockState->getTriggerOffset(Const::EDetector::ECL,
            "fitting") << " [ns] " << std::endl;

}
