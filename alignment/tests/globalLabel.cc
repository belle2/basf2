#include <alignment/GlobalLabel.h>
#include <alignment/PedeResult.h>

#include <vxd/dataobjects/VxdID.h>
#include <cdc/dataobjects/WireID.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <framework/database/Database.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include <TClonesArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <alignment/dbobjects/VXDAlignment.h>


using namespace std;
using namespace Belle2;
using namespace alignment;

namespace Belle2 {
  /** Test fixture. */
  class GlobalLabelTest : public ::testing::Test {
  protected:
    /// Testing vxd id
    VxdID vxdid;
    /// Testing cdc id
    WireID cdcid;
    /// Testng parameter id
    int paramid;

    /** init */
    virtual void SetUp()
    {
      vxdid = VxdID(1, 2, 1);
      cdcid = WireID(1, 4, 60);
      paramid = 9;
    }

    /// Register some parameters as time dependent
    void registerSomeTimeDepParams()
    {
      GlobalLabel(vxdid, paramid).registerTimeDependent(1, 100); // timeid=1 for 1..100
      GlobalLabel(cdcid, paramid).registerTimeDependent(1, 50); // timeid=1 for 1..50
      GlobalLabel(cdcid, paramid).registerTimeDependent(51, 100); // timeid=51 for 51..100
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
        GlobalLabel lab(id, 1);
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
        GlobalLabel movingLayer(VxdID(ilayer, 0, 0), 2);
        GlobalLabel movingLayerStaticParam(VxdID(ilayer, 0, 0), 4);
        GlobalLabel staticLayer(VxdID(6, 0, 0), 2);

        EXPECT_EQ(i, movingLayer.getTimeId());
        EXPECT_EQ(0, movingLayerStaticParam.getTimeId());
        EXPECT_EQ(0, staticLayer.getTimeId());
      }
    }

  }

  /// Test that time dependence works
  TEST_F(GlobalLabelTest, TimeSettingWorkflow)
  {
    GlobalLabel vxdlabel1(vxdid, paramid);
    GlobalLabel Cvxdlabel1(vxdid, 2);
    EXPECT_EQ(0, vxdlabel1.getTimeId());
    EXPECT_EQ(0, Cvxdlabel1.getTimeId());

    registerSomeTimeDepParams();
    GlobalLabel vxdlabel2(vxdid, paramid);
    GlobalLabel Cvxdlabel2(vxdid, 2);
    EXPECT_EQ(0, vxdlabel2.getTimeId());
    EXPECT_EQ(0, Cvxdlabel2.getTimeId());

    setTime(80);
    GlobalLabel vxdlabel3(vxdid, paramid);
    GlobalLabel Cvxdlabel3(vxdid, 2);
    EXPECT_EQ(1, vxdlabel3.getTimeId());
    EXPECT_EQ(0, Cvxdlabel3.getTimeId());

  }

  /// Test getters/setters, operators
  TEST_F(GlobalLabelTest, GettersSettersOperators)
  {
    GlobalLabel vxdlabel1(vxdid, paramid);
    GlobalLabel vxdlabel2(100873609);

    // getters
    EXPECT_EQ(80, GlobalLabel::getCurrentTimeIntervalRef());
    EXPECT_EQ(paramid, vxdlabel1.getParameterId());
    EXPECT_EQ(0, vxdlabel1.getTimeFlag());
    EXPECT_EQ(0, vxdlabel1.getTimeId());
    EXPECT_EQ(vxdid, vxdlabel1.getVxdID());
    EXPECT_EQ(65535, vxdlabel1.getWireID());
    // no detector mismatch
    EXPECT_EQ(0, GlobalLabel(cdcid, paramid).getVxdID());
    EXPECT_EQ(cdcid, GlobalLabel(cdcid, paramid).getWireID());
    // cast operators
    EXPECT_EQ(100873609, (int)vxdlabel1);
    EXPECT_EQ(100873609, (unsigned int)vxdlabel1);

    // getters (GlobalLabel from label)
    EXPECT_EQ(80, GlobalLabel::getCurrentTimeIntervalRef());
    EXPECT_EQ(paramid, vxdlabel2.getParameterId());
    EXPECT_EQ(0, vxdlabel2.getTimeFlag());
    EXPECT_EQ(0, vxdlabel2.getTimeId());
    EXPECT_EQ(vxdid, vxdlabel2.getVxdID());
    EXPECT_EQ(65535, vxdlabel2.getWireID());
    // no detector mismatch (GlobalLabel from label)
    EXPECT_EQ(0, GlobalLabel(cdcid, paramid).getVxdID());
    EXPECT_EQ(cdcid, GlobalLabel(cdcid, paramid).getWireID());
    // cats operator (GlobalLabel from label)
    EXPECT_EQ(100873609, (int)vxdlabel2);
    EXPECT_EQ(100873609, (unsigned int)vxdlabel2);

    // Assignment
    GlobalLabel other(cdcid, 20);
    other = vxdlabel1;
    EXPECT_EQ(100873609, other.label());
  }

  /// Test the default way of using this
  TEST_F(GlobalLabelTest, QickTestNormalOperation)
  {

    // Test time indep. detector constructors
    GlobalLabel vxdlabel(vxdid, paramid);
    GlobalLabel cdclabel(cdcid, paramid);
    EXPECT_EQ(100873609, vxdlabel.label());
    EXPECT_EQ(200620409, cdclabel.label());
    EXPECT_EQ(vxdid.getLadderNumber(), vxdlabel.getVxdID().getLadderNumber());
    EXPECT_EQ(cdcid.getICLayer(), cdclabel.getWireID().getICLayer());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());
    // Test time indep. label constructor
    vxdlabel = GlobalLabel(100873609);
    cdclabel = GlobalLabel(200620409);
    EXPECT_EQ(vxdid.getLadderNumber(), vxdlabel.getVxdID().getLadderNumber());
    EXPECT_EQ(cdcid.getICLayer(), cdclabel.getWireID().getICLayer());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());

    // Register time intervals
    registerSomeTimeDepParams();

    // Nothing to test on dump... (enable it and see the time-dep map:-)
    //vxdlabel.dump(100);

    // Test detector time dependent constructors
    setTime(10);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(1, cdclabel.getTimeId());
    EXPECT_EQ(vxdid.getLadderNumber(), vxdlabel.getVxdID().getLadderNumber());
    EXPECT_EQ(cdcid.getICLayer(), cdclabel.getWireID().getICLayer());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());

    // Test time dependent label constructor
    vxdlabel = GlobalLabel(vxdlabel.label());
    cdclabel = GlobalLabel(cdclabel.label());
    EXPECT_EQ(vxdid.getLadderNumber(), vxdlabel.getVxdID().getLadderNumber());
    EXPECT_EQ(cdcid.getICLayer(), cdclabel.getWireID().getICLayer());
    EXPECT_EQ(paramid, vxdlabel.getParameterId());
    EXPECT_EQ(paramid, cdclabel.getParameterId());
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(1, cdclabel.getTimeId());

    // Test that time dependence works correctly
    // on several subruns
    setTime(0);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(0, vxdlabel.getTimeId());
    EXPECT_EQ(0, cdclabel.getTimeId());


    setTime(1);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(1, cdclabel.getTimeId());

    setTime(51);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(51, cdclabel.getTimeId());

    setTime(80);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(51, cdclabel.getTimeId());

    setTime(100);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(1, vxdlabel.getTimeId());
    EXPECT_EQ(51, cdclabel.getTimeId());

    setTime(101);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(0, vxdlabel.getTimeId());
    EXPECT_EQ(0, cdclabel.getTimeId());

    // Reset time dependence
    GlobalLabel::clearTimeDependentParamaters();
    setTime(90);
    vxdlabel = GlobalLabel(vxdid, paramid);
    cdclabel = GlobalLabel(cdcid, paramid);
    EXPECT_EQ(0, vxdlabel.getTimeId());
    EXPECT_EQ(0, cdclabel.getTimeId());

  }
}  // namespace
