/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>

#include <boost/python.hpp>
#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, cpp_storeWrapper)
{
  StoreObjPtr<StoreWrapper<std::vector<CDCWireHit> > > stdVector;
  // do something so that cppcheck is happy
  stdVector.isOptional();
}

TEST(TrackFindingCDCTest, cpp_storeWrappedObjPtr)
{
  StoreWrappedObjPtr<std::vector<CDCWireHit> > stdVector;
  // do something so that cppcheck is happy
  stdVector.isOptional();
}

template <class T>
class DISABLED_TrackFindingCDCTestRootification : public TrackFindingCDCTestWithTopology {
};

using RootifiedTypes =
  ::testing::Types<std::vector<CDCWireHit>,
  std::vector<CDCSegment2D>,
  std::vector<CDCTrack>,
  std::vector<WeightedRelation<const CDCTrack> > >;

TYPED_TEST_SUITE(DISABLED_TrackFindingCDCTestRootification, RootifiedTypes);

TYPED_TEST(DISABLED_TrackFindingCDCTestRootification, rootification_UsableAsStoreWrappedObjPtr)
{
  DataStore& datastore = DataStore::Instance();
  StoreWrappedObjPtr< TypeParam > storeObj;

  // Initialization phase
  datastore.setInitializeActive(true);
  storeObj.registerInDataStore(DataStore::c_DontWriteOut);
  datastore.setInitializeActive(false);

  // Event processing phase
  // First module
  storeObj.create();
  const std::string& objectName = storeObj.getName();

  // Second module - a python module
  // Setting up the Python interpreter
  Py_Initialize(); // Repeated initialization calls do nothing

  // Loading basf2
  ASSERT_NE(-1, PyRun_SimpleString("import basf2"));
  ASSERT_NE(-1, PyRun_SimpleString("from ROOT import Belle2"));

  // Make the object name available in the Python interpreter
  std::string pyCmd_transferName_prototype = "objectName = 'X'";
  std::string pyCmd_transferName =  pyCmd_transferName_prototype.replace(pyCmd_transferName_prototype.find("X"), 1, objectName);
  ASSERT_NE(-1, PyRun_SimpleString(pyCmd_transferName.c_str()));

  // Fetch the object from the DataStore and use it
  ASSERT_NE(-1, PyRun_SimpleString("pyStoreObj = Belle2.PyStoreObj(objectName)"));
  ASSERT_NE(-1, PyRun_SimpleString("if not pyStoreObj: raise ValueError('PyStoreObj does not point to a "
                                   "valid object on the DataStore')"));
  ASSERT_NE(-1, PyRun_SimpleString("print('pyStoreObj.obj() is', pyStoreObj.obj())"));
  ASSERT_NE(-1, PyRun_SimpleString("print('pyStoreObj.obj().unwrap() is', pyStoreObj.obj().unwrap())"));

  // Note: if an error is raised in Python the return value is -1
  // ASSERT_EQ(-1,PyRun_SimpleString("raise ValueError('This is a test')"));

  // Root crashes, if you terminate the python interpreter already
  // Py_Finalize();
}
