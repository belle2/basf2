/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/GlobalTimeLine.h>
#include <alignment/Manager.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <framework/database/Configuration.h>
#include <framework/database/Database.h>
#include <framework/database/EventDependency.h>
#include <framework/database/TestingPayloadStorage.h>
#include <framework/dataobjects/EventMetaData.h>
#include <vxd/dataobjects/VxdID.h>

#include <gtest/gtest.h>

#include <TFile.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <string>

using namespace std;
using namespace Belle2;
using namespace alignment;
using namespace timeline;

namespace {
  template<int UID>
  class MockDBObj {
  public:
    MockDBObj(double value = 0.) : m_param(value) {}
    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID()
    {
      static_assert(UID != 0, "Non-zero UID required");
      return UID;
    }
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short param) {return m_param;}
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short param) {m_param = value;}
    /// TODO: list stored global parameters
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {{0, 0}};}
  private:
    double m_param{0.};
  };


  class TimeLineTest : public ::testing::Test {
  protected:

    IntraIoVPayloads beamspotRun0 = {
      {EventMetaData(0, 0, 0), std::make_shared<GlobalParamSet<BeamSpot>>()}
    };

    IntraIoVPayloads beamspotRun1 = {
      {EventMetaData(0, 1, 0), std::make_shared<GlobalParamSet<BeamSpot>>()}
    };

    IntraIoVPayloads beamspotRun2 = {
      {EventMetaData(0, 2, 0), std::make_shared<GlobalParamSet<BeamSpot>>()},
      {EventMetaData(530532, 2, 0), std::make_shared<GlobalParamSet<BeamSpot>>()}
    };

    IntraIoVPayloads beamspotRun3 = {
      {EventMetaData(0, 3, 0), std::make_shared<GlobalParamSet<BeamSpot>>()}
    };

    IntraIoVPayloads beamspotRun4 = {
      {EventMetaData(0, 4, 0), std::make_shared<GlobalParamSet<BeamSpot>>()}
    };

    PayloadIovBlockRow beamspotRow = {
      {IntervalOfValidity(0, 0, 0, 0), beamspotRun0},
      {IntervalOfValidity(0, 1, 0, 1), beamspotRun1},
      {IntervalOfValidity(0, 2, 0, 2), beamspotRun2},
      {IntervalOfValidity(0, 3, 0, 3), beamspotRun3},
      {IntervalOfValidity(0, 4, 0, 4), beamspotRun4}
    };

    IntraIoVPayloads vxdRun0to2 = {
      {EventMetaData(0, 0, 0), std::make_shared<GlobalParamSet<VXDAlignment>>()},
    };

    IntraIoVPayloads vxdRun3toInf = {
      {EventMetaData(0, 3, 0), std::make_shared<GlobalParamSet<VXDAlignment>>()},
    };

    PayloadIovBlockRow vxdRow = {
      {IntervalOfValidity(0, 0, 0, 2), vxdRun0to2},
      {IntervalOfValidity(0, 3, 0, 4), vxdRun3toInf}

    };

    IntraIoVPayloads cdcBlock = {
      {EventMetaData(0, 0, 0), std::make_shared<GlobalParamSet<CDCAlignment>>()}

    };
    PayloadIovBlockRow cdcRow = {
      {IntervalOfValidity(0, 0, 0, 4), cdcBlock}
    };

    PayloadsTable payloadsTable = {
      {BeamSpot::getGlobalUniqueID(), beamspotRow},
      {VXDAlignment::getGlobalUniqueID(), vxdRow},
      {CDCAlignment::getGlobalUniqueID(), cdcRow}
    };

    std::vector<int> head0 = {0, 1, 2, 3, 4, 5};
    std::vector<int> head1 = {0, 1, 2, 2, 3, 4};
    std::vector<int> row0  = {0, 1, 2, 3, 4, 5};
    std::vector<int> row1  = {0, 0, 0, 0, 1, 1};
    std::vector<int> row2  = {0, 0, 0, 0, 0, 0};

    EventHeader eventHeader = {
      EventMetaData(0, 0, 0),
      EventMetaData(0, 1, 0),
      EventMetaData(0, 2, 0),
      EventMetaData(530532, 2, 0),
      EventMetaData(0, 3, 0),
      EventMetaData(0, 4, 0)
    };

    RunHeader runHeader = {
      {0, 0},
      {0, 1},
      {0, 2},
      {0, 2},
      {0, 3},
      {0, 4}
    };

    TableData tableData = {
      { 1, row0},
      {10, row1},
      {27, row2}
    };

    TimeTable timeTable = std::make_tuple(eventHeader, runHeader, tableData);

    std::vector<int> irow0  = {0, 1, 1, 1, 1, 1};
    std::vector<int> irow1  = {0, 0, 0, 0, 1, 0};
    std::vector<int> irow2  = {0, 0, 0, 0, 0, 0};

    TableData initTableData = {
      { 1, irow0},
      {10, irow1},
      {27, irow2}
    };

    TimeTable initTimeTable = std::make_tuple(eventHeader, runHeader, initTableData);

    /// init
    virtual void SetUp()
    {
      StoreObjPtr<EventMetaData> evtPtr;
      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      // Event 0 run 0 exp 0
      evtPtr.construct(0, 0, 0);

      auto& c = Conditions::Configuration::getInstance();
      c.setNewPayloadLocation("testPayloads/TestDatabase.txt");

      GlobalLabel::setCurrentTimeInterval(0);
      GlobalLabel::clearTimeDependentParamaters();

      auto beamspotX = GlobalLabel::construct<BeamSpot>(0, 1);
      auto beamspotZ = GlobalLabel::construct<BeamSpot>(0, 3);
      auto yingZ = GlobalLabel::construct<VXDAlignment>(VxdID(1, 0, 0, 1).getID(), 3);

      yingZ.registerTimeDependent(4);

      // should be banned:
      //beamspotX.registerTimeDependent(3);
      // -> instead:
      beamspotX.registerTimeDependent(2);
      beamspotX.registerTimeDependent(3);
      beamspotX.registerTimeDependent(4);

      beamspotZ.registerTimeDependent(1);
      beamspotZ.registerTimeDependent(2);
      beamspotZ.registerTimeDependent(3);
      beamspotZ.registerTimeDependent(4);
      beamspotZ.registerTimeDependent(5);

    }

    /// cleanup
    virtual void TearDown()
    {
      GlobalLabel::clearTimeDependentParamaters();
      GlobalLabel::setCurrentTimeInterval(0);

      boost::filesystem::remove_all("testPayloads");
      Database::reset();
      DataStore::Instance().reset();
    }

  };

  /// Test creation of initial TimeTable
  TEST_F(TimeLineTest, InitialTimeTable)
  {
    GlobalLabel label;
    auto table = makeInitialTimeTable(eventHeader, label);

    std::vector<int> myrow0  = {0, 1, 1, 1, 1, 1};
    std::vector<int> myrow1  = {0, 0, 0, 0, 1, 0};

    TableData myInitTableData = {
      { 1, myrow0},
      {10, myrow1}
    };

    TimeTable myInitTimeTable = std::make_tuple(eventHeader, runHeader, myInitTableData);


    EXPECT_EQ(table, myInitTimeTable);

  }

  /// Test finalization of the timetable and its adressing
  TEST_F(TimeLineTest, TimeTable)
  {
    finalizeTimeTable(initTimeTable);
    EXPECT_EQ(initTimeTable, timeTable);

    EXPECT_EQ(getContinuousIndexByTimeID(timeTable, 1,  3), 3);
    EXPECT_EQ(getContinuousIndexByTimeID(timeTable, 10, 3), 0);
    EXPECT_EQ(getContinuousIndexByTimeID(timeTable, 27, 3), 0);

  }

  /// Test calculation of global derivatives
  TEST_F(TimeLineTest, PayloadTable)
  {

    getPayloadByContinuousIndex(payloadsTable, 1, 3).second->setGlobalParam(42., 0, 1);
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 3).second->getGlobalParam(0, 1), 42.);


    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 0).first, EventMetaData(0, 0, 0));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 1).first, EventMetaData(0, 1, 0));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 2).first, EventMetaData(0, 2, 0));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 3).first, EventMetaData(530532, 2, 0));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 4).first, EventMetaData(0, 3, 0));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 1, 5).first, EventMetaData(0, 4, 0));

    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 10, 0).first, EventMetaData(0, 0, 0));
    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 10, 1).first, EventMetaData(0, 3, 0));

    EXPECT_EQ(getPayloadByContinuousIndex(payloadsTable, 27, 0).first, EventMetaData(0, 0, 0));


  }

  /// Test creation of the payloads table from finalized timetable
  TEST_F(TimeLineTest, TimeTable2PayloadsTable)
  {
    GlobalParamVector vector({"BeamSpot", "VXDAlignment", "CDCAlignment"});
    GlobalCalibrationManager::initGlobalVector(vector);

    int timeid = 0;
    EventMetaData ev;

    // vxd
    timeid = 0;
    ev = gotoNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 0);
    EXPECT_EQ(ev, EventMetaData(0, 0, 0));

    timeid = 1;
    ev = gotoNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 1);
    EXPECT_EQ(ev, EventMetaData(0, 1, 0));

    timeid = 4;
    ev = gotoNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 4);
    EXPECT_EQ(ev, EventMetaData(0, 3, 0));

    timeid = 5;
    ev = gotoNextChangeInRun(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 0));

    // beam
    timeid = 0;
    ev = gotoNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 0);
    EXPECT_EQ(ev, EventMetaData(0, 0, 0));

    timeid = 1;
    ev = gotoNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 1);
    EXPECT_EQ(ev, EventMetaData(0, 1, 0));

    timeid = 2;
    ev = gotoNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 0));

    timeid = 3;
    ev = gotoNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(530532, 2, 0));

    timeid = 4;
    ev = gotoNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 4);
    EXPECT_EQ(ev, EventMetaData(0, 3, 0));

    timeid = 5;
    ev = gotoNextChangeInRun(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 0));



    timeid = 0;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 0);
    EXPECT_EQ(ev, EventMetaData(0, 1, 0));

    timeid = 1;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 1);
    EXPECT_EQ(ev, EventMetaData(0, 2, 0));

    timeid = 2;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(0, 3, 0));

    timeid = 3;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(0, 3, 0));

    timeid = 4;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 4);
    EXPECT_EQ(ev, EventMetaData(0, 4, 0));

    timeid = 5;
    ev = gotoNextChangeRunWise(timeTable, 1, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 0));



    timeid = 0;
    ev = gotoNextChangeRunWise(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(0, 3, 0));
    ev = gotoNextChangeRunWise(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 3);
    EXPECT_EQ(ev, EventMetaData(0, 3, 0));

    timeid = 4;
    ev = gotoNextChangeRunWise(timeTable, 10, timeid);
    EXPECT_EQ(timeid, 5);
    EXPECT_EQ(ev, EventMetaData(0, 4, 0));




    auto table = TimeIdsTable2PayloadsTable(timeTable, vector);


    EXPECT_EQ(getPayloadByContinuousIndex(table, 27, 0).first.getExperiment(), getPayloadByContinuousIndex(payloadsTable, 27,
              0).first.getExperiment());


    B2INFO(table.at(27).at(0).second.at(0).first.getEvent() << " " << table.at(27).at(0).second.at(0).first.getRun() << " " << table.at(
             27).at(0).second.at(0).first.getExperiment());
    EXPECT_EQ(table.at(27).at(0).second.at(0).first, payloadsTable.at(27).at(0).second.at(0).first);
    B2INFO(table.at(27).at(0).second.at(0).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(27).at(0).second.at(0).second->getGlobalUniqueID(),
              payloadsTable.at(27).at(0).second.at(0).second->getGlobalUniqueID());

    B2INFO(table.at(10).at(0).second.at(0).first.getEvent() << " " << table.at(10).at(0).second.at(0).first.getRun() << " " << table.at(
             10).at(0).second.at(0).first.getExperiment());
    EXPECT_EQ(table.at(10).at(0).second.at(0).first, payloadsTable.at(10).at(0).second.at(0).first);
    B2INFO(table.at(10).at(0).second.at(0).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(10).at(0).second.at(0).second->getGlobalUniqueID(),
              payloadsTable.at(10).at(0).second.at(0).second->getGlobalUniqueID());

    B2INFO(table.at(1).at(0).second.at(0).first.getEvent() << " " << table.at(1).at(0).second.at(0).first.getRun() << " " << table.at(
             1).at(0).second.at(0).first.getExperiment());
    EXPECT_EQ(table.at(1).at(0).second.at(0).first, payloadsTable.at(1).at(0).second.at(0).first);
    B2INFO(table.at(1).at(0).second.at(0).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(1).at(0).second.at(0).second->getGlobalUniqueID(),
              payloadsTable.at(1).at(0).second.at(0).second->getGlobalUniqueID());

    B2INFO(table.at(1).at(2).second.at(1).first.getEvent() << " " << table.at(1).at(2).second.at(1).first.getRun() << " " << table.at(
             1).at(2).second.at(1).first.getExperiment());
    EXPECT_EQ(table.at(1).at(2).second.at(1).first, payloadsTable.at(1).at(2).second.at(1).first);
    B2INFO(table.at(1).at(2).second.at(1).second->getGlobalUniqueID());
    EXPECT_EQ(table.at(1).at(2).second.at(1).second->getGlobalUniqueID(),
              payloadsTable.at(1).at(2).second.at(1).second->getGlobalUniqueID());


  }

  /// Test the full workflow using GlobalParamTimeLin
  TEST_F(TimeLineTest, GlobalParamTimeLine)
  {

    GlobalParamVector vector({"BeamSpot", "VXDAlignment", "CDCAlignment"});
    GlobalCalibrationManager::initGlobalVector(vector);
    GlobalLabel label;
    GlobalParamTimeLine timeline(eventHeader, label, vector);

    timeline.loadFromDB();

    GlobalLabel::setCurrentTimeInterval(4);
    auto beamX0 = GlobalLabel::construct<BeamSpot>(0, 1).label();
    timeline.updateGlobalParam(GlobalLabel(beamX0), 3.);

    GlobalLabel::setCurrentTimeInterval(3);
    auto beamZ = GlobalLabel::construct<BeamSpot>(0, 3).label();
    auto beamX = GlobalLabel::construct<BeamSpot>(0, 1).label();

    GlobalLabel::setCurrentTimeInterval(4);
    auto yingZ = GlobalLabel::construct<VXDAlignment>(VxdID(1, 0, 0, 1).getID(), 3).label();


    timeline.updateGlobalParam(GlobalLabel(beamZ), 42.);
    timeline.updateGlobalParam(GlobalLabel(beamX), 43.);
    timeline.updateGlobalParam(GlobalLabel(yingZ), 44.);

    auto objects = timeline.releaseObjects();
    EXPECT_EQ(objects.size(), 8);

    for (auto iov_obj : objects)
      Database::Instance().storeData(DataStore::objectName(iov_obj.second->IsA(), ""), iov_obj.second, iov_obj.first);

    // Ok let's open these payload files manually to see what's in them but at least we use the framework functions to
    // find the correct payload for any given iov
    Conditions::TestingPayloadStorage payloads("testPayloads/TestDatabase.txt");
    {
      EventMetaData eventMetaData(0, 2);
      Conditions::PayloadMetadata payloadInfo{"EventDependency"};
      ASSERT_TRUE(payloads.get(eventMetaData, payloadInfo));
      ASSERT_FALSE(payloadInfo.filename.empty());
      TFile file(payloadInfo.filename.c_str());
      ASSERT_TRUE(file.IsOpen());
      auto evdep = (EventDependency*) file.Get("EventDependency");
      ASSERT_TRUE(evdep);

      auto beam = dynamic_cast<BeamSpot*>(evdep->getObject(EventMetaData(530532, 2, 0)));
      EXPECT_EQ(beam->getIPPosition()[2], 42.);

      beam = dynamic_cast<BeamSpot*>(evdep->getObject(EventMetaData(530532, 2, 0)));
      EXPECT_EQ(beam->getIPPosition()[0], 43.);

      beam = dynamic_cast<BeamSpot*>(evdep->getObject(EventMetaData(530532 - 1, 2, 0)));
      EXPECT_EQ(beam->getIPPosition()[2], 0.);

      beam = dynamic_cast<BeamSpot*>(evdep->getObject(EventMetaData(530532 - 1, 2, 0)));
      EXPECT_EQ(beam->getIPPosition()[0], 0.);
    }
    {
      EventMetaData eventMetaData(0, 3);
      Conditions::PayloadMetadata payloadInfo{"BeamSpot"};
      ASSERT_TRUE(payloads.get(eventMetaData, payloadInfo));
      ASSERT_FALSE(payloadInfo.filename.empty());
      TFile file(payloadInfo.filename.c_str());
      ASSERT_TRUE(file.IsOpen());

      auto beam2 = (BeamSpot*) file.Get("BeamSpot");
      ASSERT_TRUE(beam2);
      beam2->getIPPosition().Print();
      EXPECT_EQ(beam2->getIPPosition()[0], 3.);
    }
    {
      EventMetaData eventMetaData(0, 4);
      Conditions::PayloadMetadata payloadInfo{"VXDAlignment"};
      ASSERT_TRUE(payloads.get(eventMetaData, payloadInfo));
      ASSERT_FALSE(payloadInfo.filename.empty());
      TFile file(payloadInfo.filename.c_str());
      ASSERT_TRUE(file.IsOpen());

      auto vxd = (VXDAlignment*) file.Get("VXDAlignment");
      ASSERT_TRUE(vxd);
      EXPECT_EQ(vxd->getGlobalParam(VxdID(1, 0, 0, 1).getID(), 3), 44.);
    }
    // --------------------------------------------------------------------
  }
}  // namespace
