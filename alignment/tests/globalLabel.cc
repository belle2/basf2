/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/GlobalLabel.h>
#include <alignment/PedeResult.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <vxd/dataobjects/VxdID.h>

#include <gtest/gtest.h>

#include <iostream>
#include <string>

using namespace std;
using namespace Belle2;
using namespace alignment;

namespace Belle2 {
  /** Test fixture. */
  class GlobalLabelTest : public ::testing::Test {
  protected:
    /// Testing vxd id
    short vxdSensor;
    /// Testing cdc id
    short cdcWire;
    /// Testng parameter id
    int paramid;

    /** init */
    virtual void SetUp()
    {
      vxdSensor = VxdID(1, 2, 1).getID();
      cdcWire = WireID(1, 4, 60).getEWire();
      paramid = 9;
    }

    /// Register some parameters as time dependent
    void registerSomeTimeDepParams()
    {
      GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid).registerTimeDependent(1, 100); // timeid=1 for 1..100
      GlobalLabel::construct<CDCAlignment>(cdcWire, paramid).registerTimeDependent(1, 50); // timeid=1 for 1..50
      GlobalLabel::construct<CDCAlignment>(cdcWire, paramid).registerTimeDependent(51, 100); // timeid=51 for 51..100
    }

    /// Set current time slice for time dependent params
    void setTime(unsigned int timeid)
    {
      unsigned int& subrun = GlobalLabel::getCurrentTimeIntervalRef();
      subrun = timeid;
    }

    /** cleanup */
    virtual void TearDown()
    {
      GlobalLabel::clearTimeDependentParamaters();
    }

  };

  /// Test large number of time-dep params for registration and retrieval
  TEST_F(GlobalLabelTest, LargeNumberOfTimeDependentParameters)
  {
    for (int ilayer = 1; ilayer < 6; ilayer++) {
      for (unsigned int subrun = 0; subrun <= GlobalLabel::maxTID; subrun++) {
        VxdID id(ilayer, 0, 0);
        GlobalLabel lab = GlobalLabel::construct<VXDAlignment>(id.getID(), 1);
        lab.registerTimeDependent(subrun, subrun);
        lab.setParameterId(2);
        lab.registerTimeDependent(subrun, subrun);
        lab.setParameterId(3);
        lab.registerTimeDependent(subrun, subrun);
      }
    }
    // Ask 1000-times for time dep.label (could be in a single track)
    for (int ilayer = 1; ilayer < 6; ilayer++) {
      for (unsigned int i = 0; i <= GlobalLabel::maxTID; i++) {
        setTime(i);
        GlobalLabel movingLayer = GlobalLabel::construct<VXDAlignment>(VxdID(ilayer, 0, 0).getID(), 2);
        GlobalLabel movingLayerStaticParam = GlobalLabel::construct<VXDAlignment>(VxdID(ilayer, 0, 0).getID(), 4);
        GlobalLabel staticLayer = GlobalLabel::construct<VXDAlignment>(VxdID(6, 0, 0).getID(), 2);

        EXPECT_EQ(i, movingLayer.getTimeId());
        EXPECT_EQ(0, movingLayerStaticParam.getTimeId());
        EXPECT_EQ(0, staticLayer.getTimeId());
      }
    }

  }

  /// Test that time dependence works
  TEST_F(GlobalLabelTest, TimeSettingWorkflow)
  {
    GlobalLabel vxdlabel1 = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    GlobalLabel Cvxdlabel1 = GlobalLabel::construct<VXDAlignment>(vxdSensor, 2);
    EXPECT_EQ(0, vxdlabel1.getTimeId());
    EXPECT_EQ(0, Cvxdlabel1.getTimeId());

    registerSomeTimeDepParams();
    GlobalLabel vxdlabel2 = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    GlobalLabel Cvxdlabel2 = GlobalLabel::construct<VXDAlignment>(vxdSensor, 2);
    EXPECT_EQ(0, vxdlabel2.getTimeId());
    EXPECT_EQ(0, Cvxdlabel2.getTimeId());

    setTime(80);
    GlobalLabel vxdlabel3 = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    GlobalLabel Cvxdlabel3 = GlobalLabel::construct<VXDAlignment>(vxdSensor, 2);
    EXPECT_EQ(1, vxdlabel3.getTimeId());
    EXPECT_EQ(0, Cvxdlabel3.getTimeId());

  }

  /// Test getters/setters, operators
  TEST_F(GlobalLabelTest, GettersSettersOperators)
  {
    GlobalLabel vxdlabel1 = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    GlobalLabel vxdlabel2(100873609);

    // getters
    EXPECT_EQ(80, GlobalLabel::getCurrentTimeIntervalRef());
    EXPECT_EQ(paramid, vxdlabel1.getParameterId());
    EXPECT_EQ(0, vxdlabel1.getTimeFlag());
    EXPECT_EQ(0, vxdlabel1.getTimeId());
    EXPECT_EQ(vxdSensor, vxdlabel1.getElementId());

    // cast operators
    EXPECT_EQ(100873609, (int)vxdlabel1);
    EXPECT_EQ(100873609, (unsigned int)vxdlabel1);

    // getters (GlobalLabel from label)
    EXPECT_EQ(80, GlobalLabel::getCurrentTimeIntervalRef());
    EXPECT_EQ(paramid, vxdlabel2.getParameterId());
    EXPECT_EQ(0, vxdlabel2.getTimeFlag());
    EXPECT_EQ(0, vxdlabel2.getTimeId());
    EXPECT_EQ(vxdSensor, vxdlabel2.getElementId());

    // no detector mismatch (GlobalLabel from label)
    // cats operator (GlobalLabel from label)
    EXPECT_EQ(100873609, (int)vxdlabel2);
    EXPECT_EQ(100873609, (unsigned int)vxdlabel2);

    // Assignment
    GlobalLabel other = GlobalLabel::construct<CDCAlignment>(cdcWire, 2);
    EXPECT_EQ(270620402, other.label());
    other = vxdlabel1;
    EXPECT_EQ(100873609, other.label());
  }

  /// Test the default way of using this
  TEST_F(GlobalLabelTest, QickTestNormalOperation)
  {

    // Test time indep. detector constructors
    GlobalLabel vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    GlobalLabel cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(100873609, vxdlabel.label());
    EXPECT_EQ(270620409, cdclabel.label());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());
    // Test time indep. label constructor
    vxdlabel = GlobalLabel(100873609);
    cdclabel = GlobalLabel(270620409);
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());

    // Register time intervals
    registerSomeTimeDepParams();

    // Nothing to test on dump... (enable it and see the time-dep map:-)
    //vxdlabel.dump(100);

    // Test detector time dependent constructors
    setTime(10);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(1, cdclabel.getTimeId());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());

    // Test time dependent label constructor
    vxdlabel = GlobalLabel(vxdlabel.label());
    cdclabel = GlobalLabel(cdclabel.label());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(1, cdclabel.getTimeId());

    // Test that time dependence works correctly
    // on several subruns
    setTime(0);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(0, vxdlabel.getTimeId());
    EXPECT_EQ(0, cdclabel.getTimeId());


    setTime(1);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(1, cdclabel.getTimeId());

    setTime(51);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(51, cdclabel.getTimeId());

    setTime(80);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(51, cdclabel.getTimeId());

    setTime(100);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(51, cdclabel.getTimeId());

    setTime(101);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(0, vxdlabel.getTimeId());
    EXPECT_EQ(0, cdclabel.getTimeId());

    // Reset time dependence
    GlobalLabel::clearTimeDependentParamaters();
    setTime(90);
    vxdlabel = GlobalLabel::construct<VXDAlignment>(vxdSensor, paramid);
    cdclabel = GlobalLabel::construct<CDCAlignment>(cdcWire, paramid);
    EXPECT_EQ(0, vxdlabel.getTimeId());
    EXPECT_EQ(0, cdclabel.getTimeId());

  }
}  // namespace
