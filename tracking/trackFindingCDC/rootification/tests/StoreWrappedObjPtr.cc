/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreObjPtr.h>
#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>

#include <boost/python.hpp>
#include <gtest/gtest.h>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

using namespace std;

TEST(TrackFindingCDCTest, cpp_storeWrapper)
{
  StoreObjPtr<StoreWrapper<CDCWireHit> > storedHit;
  StoreObjPtr<StoreWrapper<std::vector<CDCWireHit> > > stdVector;
  StoreObjPtr<StoreWrapper<SortableVector<CDCWireHit> > > sortableVector;
}

TEST(TrackFindingCDCTest, cpp_storeWrappedObjPtr)
{
  StoreWrappedObjPtr<CDCWireHit> storedHit;
  StoreWrappedObjPtr<std::vector<CDCWireHit> > stdVector;
  StoreWrappedObjPtr<SortableVector<CDCWireHit> > sortableVector;
}

template <typename T>
class DISABLED_TrackFindingCDCTestRootification : public TrackFindingCDCTestWithTopology {;};

typedef ::testing::Types <
CDCWireHit,
std::vector<CDCWireHit>,
SortableVector<CDCWireHit>,
std::vector<CDCRecoSegment2D>
> RootifiedTypes;


TYPED_TEST_CASE(DISABLED_TrackFindingCDCTestRootification, RootifiedTypes);


TYPED_TEST(DISABLED_TrackFindingCDCTestRootification, rootification_UsableAsStoreWrappedObjPtr)
{
  DataStore& datastore = DataStore::Instance();

  //Initialization phase
  datastore.setInitializeActive(true);
  StoreWrappedObjPtr< TypeParam >::registerTransient();
  datastore.setInitializeActive(false);

  //Event processing phase
  // First module
  StoreWrappedObjPtr< TypeParam > storeObj;
  storeObj.create();
  const string& objectName = storeObj.getName();

  // Second module - a python module
  //Setting up the Python interpreter
  Py_Initialize(); //Repeated initialization calls do nothing

  // Loading basf2
  ASSERT_NE(-1, PyRun_SimpleString("import basf2"));
  ASSERT_NE(-1, PyRun_SimpleString("from ROOT import Belle2"));

  // Make the object name available in the Python interpreter
  string pyCmd_transferName_prototype = "objectName = 'X'";
  string pyCmd_transferName =  pyCmd_transferName_prototype.replace(pyCmd_transferName_prototype.find("X"), 1, objectName);
  ASSERT_NE(-1, PyRun_SimpleString(pyCmd_transferName.c_str()));

  // Fetch the object from the DataStore and use it
  ASSERT_NE(-1, PyRun_SimpleString("pyStoreObj = Belle2.PyStoreObj(objectName)"));
  //ASSERT_NE(-1,PyRun_SimpleString("print 'pyStoreObj is',pyStoreObj"));
  ASSERT_NE(-1, PyRun_SimpleString("if not pyStoreObj: raise ValueError('PyStoreObj does not point to a valid object on the DataStore')"));
  ASSERT_NE(-1, PyRun_SimpleString("print 'pyStoreObj.obj() is', pyStoreObj.obj()"));
  ASSERT_NE(-1, PyRun_SimpleString("print 'pyStoreObj.obj().unwrap() is', pyStoreObj.obj().unwrap()"));

  //Note: if an error is raised in Python the return value is -1
  //ASSERT_EQ(-1,PyRun_SimpleString("raise ValueError('This is a test')"));

  // Root crashes, if you terminate the python interpreter already
  //Py_Finalize();

}
