/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbihler (jakob.lettenbichler@oeaw.ac.at)      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>


#include <tracking/trackFindingVXD/FilterTools/SelectionVariableFactory.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/vectorTools/B2Vector3.h>

using namespace std;

using namespace Belle2;

namespace SelFarFactoryTests {

  /** just a small proto-container storing coordinates */
  class DummyHit {
  public:
    /** std constructor */
    DummyHit(): x(0), y(0), z(0) {};

    /** constructor*/
    DummyHit(double x, double y, double z): x(x), y(y), z(z) {};

    /** destructor */
    virtual ~DummyHit() {};

    double X() const { return x; }/**< return x */
    double Y() const { return y; }/**< return y */
    double Z() const { return z; }/**< return z */

  private:
    double x; /**< x */
    double y; /**< y */
    double z; /**< z */
  };


  /** Test class for SelectionVariableFactory object. */
  class SelVarFactoryTest : public ::testing::Test {
  protected:
  };


  /** tests if raw-names are converted accordingly */
  TEST_F(SelVarFactoryTest, TestNamesWithB2Vector3)
  {
    auto d3Ds = Distance3DSquared<Belle2::B2Vector3D>();
    EXPECT_EQ("Belle2__Distance3DSquared{Belle2__B2Vector3{double}}" , d3Ds.name());

    auto aFactory = SelectionVariableFactory<Belle2::B2Vector3D>();
    auto selVarName = aFactory.getFullVariableName("Distance3DSquared");
    EXPECT_EQ("Belle2__Distance3DSquared{Belle2__B2Vector3{double}}" , selVarName);

    // full return type written down
    std::unique_ptr<SelectionVariable<B2Vector3D, double> > newSelVariable = aFactory.getSelectionVariable("Distance3DSquared");
    EXPECT_EQ(newSelVariable->name() , d3Ds.name());
  }


  /** tests if raw-names are converted accordingly */
  TEST_F(SelVarFactoryTest, TestNamesWithDummyHit)
  {
    auto d3Ds = Distance3DSquared<DummyHit>();
    auto aFactory = SelectionVariableFactory<DummyHit>();

    EXPECT_EQ("Belle2__Distance3DSquared{SelFarFactoryTests__DummyHit}", d3Ds.name());
    EXPECT_EQ(d3Ds.name() , aFactory.getFullVariableName("Distance3DSquared"));

    // shortcut written down
    auto dummyVariable = aFactory.getSelectionVariable("Distance3DSquared");
    EXPECT_EQ(dummyVariable->name() , d3Ds.name());
  }


  /** tests if raw-names are converted accordingly */
  TEST_F(SelVarFactoryTest, TestNamesWithSpacePoint)
  {
    EXPECT_EQ("Belle2__Distance3DSquared{Belle2__SpacePoint}",
              SelectionVariableFactory<SpacePoint>().getFullVariableName("Distance3DSquared"));

    auto d3Ds = Distance3DSquared<SpacePoint>();
    auto aFactory = SelectionVariableFactory<SpacePoint>();

    EXPECT_EQ("Belle2__Distance3DSquared{Belle2__SpacePoint}", d3Ds.name());
    EXPECT_EQ(d3Ds.name() , aFactory.getFullVariableName("Distance3DSquared"));

    // shortcut written down
    auto dummyVariable = aFactory.getSelectionVariable("Distance3DSquared");
    EXPECT_EQ(dummyVariable->name() , d3Ds.name());
  }


  /** tests if I can get the correct results from a SelectionVariable I asked for */
  TEST_F(SelVarFactoryTest, TestFactory)
  {
    // home hits with coordinates:
    auto hit1 = DummyHit(1, 1, 1);
    auto hit2 = DummyHit(2, 2, 3);
    auto hit3 = DummyHit(2, 3, 4);
    auto hit4 = DummyHit(23, 42, 5);
    // some SelectionVariables:
    auto d3D = Distance3DSquared<DummyHit>();
    auto a3D = Angle3DSimple<DummyHit>();
    auto dPT = DeltaPt<DummyHit>();

    auto aFactory = SelectionVariableFactory<DummyHit>();

    // get my selectionVariables via factory:
//  auto d3D_uptr = aFactory.getSelectionVariable("Distance3DSquared");
//  auto a3D_uptr = aFactory.getSelectionVariable("Angle3DSimple");
//  auto dPT_uptr = aFactory.getSelectionVariable("DeltaPt");

    // check identical name:
//  EXPECT_EQ(d3D_uptr->name() , d3D.name());
//  EXPECT_EQ(a3D_uptr->name() , a3D.name());
//  EXPECT_EQ(dPT_uptr->name() , dPT.name());

    // check identical results:
//  EXPECT_DOUBLE_EQ(d3D_uptr->value(hit1, hit2) , d3D.value(hit1, hit2));
//  EXPECT_DOUBLE_EQ(a3D_uptr->value(hit1, hit2, hit3), a3D.value(hit1, hit2, hit3));
//  EXPECT_DOUBLE_EQ(dPT_uptr->value(hit1, hit2, hit3, hit4), dPT.value(hit1, hit2, hit3, hit4));

    EXPECT_DOUBLE_EQ(aFactory.getResult2Hit("Distance3DSquared", hit1, hit2) , d3D.value(hit1, hit2));
    EXPECT_DOUBLE_EQ(aFactory.getResult3Hit("Angle3DSimple", hit1, hit2, hit3), a3D.value(hit1, hit2, hit3));
    EXPECT_DOUBLE_EQ(aFactory.getResult4Hit("DeltaPt", hit1, hit2, hit3, hit4), dPT.value(hit1, hit2, hit3, hit4));
  }

}
