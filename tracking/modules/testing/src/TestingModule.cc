/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/testing/TestingModule.h>


/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackingUtilities/rootification/StoreWrapper.h>

#include <iostream>

using namespace Belle2;

REG_MODULE(Testing);

TestingModule::TestingModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(simple module to test datastore io)DOC");

  // Parameter definitions

}

void TestingModule::initialize()
{

  std::cout << "initialize" << std::endl;

  Belle2::TrackingUtilities::StoreWrappedObjPtr< std::vector<unsigned int> > storeVector("TestBadBoardsVector");
  storeVector.registerInDataStore();
}

void TestingModule::beginRun()
{
}

void TestingModule::event()
{
  Belle2::TrackingUtilities::StoreWrappedObjPtr< std::vector<unsigned int> > storeVector("TestBadBoardsVector");

  storeVector.create();

  storeVector->push_back(62);
  storeVector->push_back(63);
}

void TestingModule::endRun()
{
}

void TestingModule::terminate()
{
}


