/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <iostream>

#include <calibration/example_caf_lib/modules/TestCalibDBAccess/TestCalibDBAccessModule.h>
#include <framework/core/Module.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TestCalibDBAccess)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TestCalibDBAccessModule::TestCalibDBAccessModule() : Module()
{
  // Set module properties
  setDescription("Testing DB objects made in Calibration");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void TestCalibDBAccessModule::initialize()
{
}

TestCalibDBAccessModule::~TestCalibDBAccessModule()
{
}

void TestCalibDBAccessModule::event()
{
  cout << "TestCalibMean mean value = " << m_obj_constant->getMean() << "\n";
  cout << "TestCalibMean mean error value = " << m_obj_constant->getMeanError() << "\n";
  cout << "TestCalibObject[0] value = " << m_array_constants[0]->getConstantValue() << "\n";
  cout << "TestCalibObject[1] value = " << m_array_constants[1]->getConstantValue() << "\n";
}
