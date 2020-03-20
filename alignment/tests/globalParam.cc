
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/GlobalParam.h>
#include <alignment/Manager.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCXtRelations.h>
#include <framework/dataobjects/EventMetaData.h>
#include <klm/dbobjects/bklm/BKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMAlignment.h>

#include <gtest/gtest.h>

#include <iostream>
#include <string>

using namespace std;
using namespace Belle2;
using namespace alignment;

namespace {
  /// Test class for hierarchy parameter storage
//   class MockDBObj : public TObject {
//   public:
//     static unsigned short getGlobalUniqueID() { return 1; }
//     double getGlobalParam(unsigned short element, unsigned short param)
//     {
//       if (element != 1)
//         return 0.;
//       if (param != 1)
//         return 0.;
//
//       return m_value;
//     }
//     void setGlobalParam(unsigned short element, unsigned short param, double val)
//     {
//       if (element != 1)
//         return;
//       if (param != 1)
//         return;
//       m_value = val;
//     }
//     std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {{1, 1}};}
//   private:
//     double m_value {0.};
//   };

  class MockDetectorInterface : public IGlobalParamInterface {
  public:
    explicit MockDetectorInterface(int id) : m_id(id) {}
    int hasBeenCalled() { return m_called; }
    virtual void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>&,
                                GlobalParamVector&) override final
    {
      std::cout << " Interface " << m_id << ": called readFromResult()" << std::endl;
      ++m_called;
    }
  private:
    int m_id { -1};
    int m_called {0};
  };

  /// Test fixture.for hierarchy
  class GlobalParamTest : public ::testing::Test {
  protected:

    /// init - fill hierarchy
    virtual void SetUp()
    {

    }

    /// cleanup
    virtual void TearDown()
    {

    }

  };

  /// Test construction of GlobalParamVector from DB components
  /// and handling of subdetector interfaces related to DB objects
  /// or generic (included for any components configuration)
  TEST_F(GlobalParamTest, GlobalParamComponentsInterfaces)
  {
    // Full vector
    GlobalParamVector gpv;
    // Vector restricted to BeamSpot
    GlobalParamVector gpvComp({"BeamSpot"});

    // Interfaces for testing
    auto interface1 = new MockDetectorInterface(1);
    auto interface2 = new MockDetectorInterface(2);
    auto interface3 = new MockDetectorInterface(3);

    auto iptr1 = std::shared_ptr<IGlobalParamInterface>(interface1);
    auto iptr2 = std::shared_ptr<IGlobalParamInterface>(interface2);
    auto iptr3 = std::shared_ptr<IGlobalParamInterface>(interface3);

    // Init the vectors with db objects and interfaces (see GlobalCalibrationManager::initGlobalVector(...))
    for (auto vect : std::vector<GlobalParamVector*>({&gpv, &gpvComp})) {
      vect->addDBObj<BeamSpot>(iptr1);
      vect->addDBObj<VXDAlignment>(iptr2);
      vect->addDBObj<CDCAlignment>(iptr2);
      vect->addDBObj<CDCTimeZeros>();
      vect->addDBObj<CDCTimeWalks>();
      vect->addDBObj<CDCXtRelations>();
      vect->addDBObj<BKLMAlignment>();
      vect->addDBObj<EKLMAlignment>();

      vect->addSubDetectorInterface(iptr3);
      vect->addSubDetectorInterface(iptr3);
      vect->addSubDetectorInterface(iptr3);
    }

    // First a check, no interface called yet
    EXPECT_EQ(interface1->hasBeenCalled() , 0);
    EXPECT_EQ(interface2->hasBeenCalled() , 0);
    EXPECT_EQ(interface3->hasBeenCalled() , 0);

    std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>> emptyResult;

    // Full vector accepted everything and calls all interfaces
    gpv.postReadFromResult(emptyResult);
    EXPECT_EQ(interface1->hasBeenCalled() , 1);
    EXPECT_EQ(interface2->hasBeenCalled() , 1); // only one more read due to uniqueness
    EXPECT_EQ(interface3->hasBeenCalled() , 1);

    // BeamSpot have interface 1 and interface 3 (added for any components configuration)
    // Interface 2 should not be stored/called in 'gpvComp'
    gpvComp.postReadFromResult(emptyResult);
    EXPECT_EQ(interface1->hasBeenCalled() , 2);
    EXPECT_EQ(interface2->hasBeenCalled() , 1);
    EXPECT_EQ(interface3->hasBeenCalled() , 2);

    EXPECT_EQ(gpv.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    // Construct the DB objects to be able to check pointers to them
    gpv.construct();
    gpvComp.construct();

    // Full vector contains all objects
    EXPECT_EQ(!!gpv.getDBObj<VXDAlignment>(), true);
    EXPECT_EQ(!!gpv.getDBObj<BeamSpot>(), true);
    // BeamSpot global vector should only containt BeamSpot
    EXPECT_EQ(!!gpvComp.getDBObj<VXDAlignment>(), false);
    EXPECT_EQ(!!gpvComp.getDBObj<BeamSpot>(), true);

    // Full vector contains all objects
    EXPECT_EQ(gpv.getGlobalParamSet<VXDAlignment>().isConstructed(), true);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamSpot>().isConstructed(), true);
    // BeamSpot global vector should only containt BeamSpot
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    EXPECT_EQ(gpvComp.getGlobalParamSet<BeamSpot>().isConstructed(), true);

    // Accessing non-existing object will return an empty set
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().is<VXDAlignment>(), false);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().is<EmptyGlobalParamSet>(), true);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().empty(), true);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    EXPECT_EQ((bool) gpvComp.getGlobalParamSet<VXDAlignment>(), false);

    EXPECT_EQ(gpv.getGlobalParamSet<BeamSpot>().is<BeamSpot>(), true);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamSpot>().is<EmptyGlobalParamSet>(), false);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamSpot>().empty(), false);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamSpot>().isConstructed(), true);
    EXPECT_EQ((bool) gpv.getGlobalParamSet<BeamSpot>(), true);


    EXPECT_EQ(gpvComp.getGlobalParamSet<EmptyGlobalParamSet>().is<EmptyGlobalParamSet>(), true);

    // This commented because BeamSpot is not yet in the DB. Instead, for now
    // test VXDAlignment and once I get to this back, I want to have test for
    // all supported db objects.
    /*
    GlobalParamVector newgpv({"BeamSpot", "CDCAlignment"});
    GlobalCalibrationManager::initGlobalVector(newgpv);

    EXPECT_EQ(newgpv.getGlobalParamSet<BeamSpot>().isConstructed(), false);
    newgpv.construct();
    EXPECT_EQ(newgpv.getGlobalParamSet<BeamSpot>().isConstructed(), true);

    newgpv.setGlobalParam(42., BeamSpot::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(BeamSpot::getGlobalUniqueID(), 0, 1), 42.);
    newgpv.loadFromDB(EventMetaData(1, 0, 0));
    EXPECT_EQ(newgpv.getGlobalParam(BeamSpot::getGlobalUniqueID(), 0, 1), 0.);

    newgpv.updateGlobalParam(42., BeamSpot::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(BeamSpot::getGlobalUniqueID(), 0, 1), 42.);

    newgpv.setGlobalParam(42., CDCAlignment::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(CDCAlignment::getGlobalUniqueID(), 0, 1), 42.);
    */

    GlobalParamVector newgpv({"VXDAlignment", "CDCAlignment"});
    GlobalCalibrationManager::initGlobalVector(newgpv);

    EXPECT_EQ(newgpv.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    newgpv.construct();
    EXPECT_EQ(newgpv.getGlobalParamSet<VXDAlignment>().isConstructed(), true);

    newgpv.setGlobalParam(42., VXDAlignment::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(VXDAlignment::getGlobalUniqueID(), 0, 1), 42.);
    newgpv.loadFromDB(EventMetaData(1, 0, 0));
    EXPECT_EQ(newgpv.getGlobalParam(VXDAlignment::getGlobalUniqueID(), 0, 1), 0.);

    newgpv.updateGlobalParam(42., VXDAlignment::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(VXDAlignment::getGlobalUniqueID(), 0, 1), 42.);

    newgpv.setGlobalParam(42., CDCAlignment::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(CDCAlignment::getGlobalUniqueID(), 0, 1), 42.);

  }

}  // namespace
