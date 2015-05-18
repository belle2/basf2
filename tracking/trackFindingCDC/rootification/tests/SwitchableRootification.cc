/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>

// Hit objects rely on the presence of wire that they are based on. Need to load the CDCGeometry from the Gearbox.
#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <boost/python.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

#ifdef TRACKFINDINGCDC_USE_ROOT_BASE

template <typename T>
class TrackFindingCDCTestRootification : public TrackFindingCDCTestWithTopology {;};

typedef ::testing::Types <
Vector2D
// Vector3D,
// CDCGenHit,
// CDCGenHitVector<CDCGenHit>,
// CDCGenHitVector<CDCWireHit>,
// CDCWireHitVector,
// CDCRecoHit2DVector,
// CDCTangentVector,
// CDCFacetVector,
// CDCRecoHit3DVector
> RootifiedTypes;


TYPED_TEST_CASE(TrackFindingCDCTestRootification, RootifiedTypes);

TYPED_TEST(TrackFindingCDCTestRootification, rootification_UsableAsStoreObject)
{

  DataStore& datastore = DataStore::Instance();

  //Initialization phase
  datastore.setInitializeActive(true);
  StoreObjPtr< TypeParam >::registerPersistent();
  datastore.setInitializeActive(false);

  //Event processing phase
  // First module
  StoreObjPtr< TypeParam > storeObj;
  storeObj.create();
  const string&  objectName = storeObj.getName();

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
  ASSERT_NE(-1,
            PyRun_SimpleString("if not pyStoreObj: raise ValueError('PyStoreObj does not point to a valid object on the DataStore')"));
  ASSERT_NE(-1, PyRun_SimpleString("print 'pyStoreObj.obj() is', pyStoreObj.obj()"));

  //Note: if an error is raised in Python the return value is -1
  //ASSERT_EQ(-1,PyRun_SimpleString("raise ValueError('This is a test')"));

  // Root crashes, if you terminate the python interpreter already
  //Py_Finalize();

}

#endif

