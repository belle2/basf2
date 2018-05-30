#include <alignment/GlobalParam.h>
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
#include <alignment/Hierarchy.h>
#include <alignment/Manager.h>
#include <TMath.h>



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
    MockDetectorInterface(int id) : m_id(id) {}
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
    // Vector restricted to BeamParameters
    GlobalParamVector gpvComp({"BeamParameters"});

    // Interfaces for testing
    auto interface1 = new MockDetectorInterface(1);
    auto interface2 = new MockDetectorInterface(2);
    auto interface3 = new MockDetectorInterface(3);

    auto iptr1 = std::shared_ptr<IGlobalParamInterface>(interface1);
    auto iptr2 = std::shared_ptr<IGlobalParamInterface>(interface2);
    auto iptr3 = std::shared_ptr<IGlobalParamInterface>(interface3);

    // Init the vectors with db objects and interfaces (see GlobalCalibrationManager::initGlobalVector(...))
    for (auto vect : std::vector<GlobalParamVector*>({&gpv, &gpvComp})) {
      vect->addDBObj<BeamParameters>(iptr1);
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

    // BeamParameters have interface 1 and interface 3 (added for any components configuration)
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
    EXPECT_EQ(!!gpv.getDBObj<BeamParameters>(), true);
    // BeamParameters global vector should only containt BeamParameters
    EXPECT_EQ(!!gpvComp.getDBObj<VXDAlignment>(), false);
    EXPECT_EQ(!!gpvComp.getDBObj<BeamParameters>(), true);

    // Full vector contains all objects
    EXPECT_EQ(gpv.getGlobalParamSet<VXDAlignment>().isConstructed(), true);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamParameters>().isConstructed(), true);
    // BeamParameters global vector should only containt BeamParameters
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    EXPECT_EQ(gpvComp.getGlobalParamSet<BeamParameters>().isConstructed(), true);

    // Accessing non-existing object will return an empty set
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().is<VXDAlignment>(), false);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().is<EmptyGlobalParamSet>(), true);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().empty(), true);
    EXPECT_EQ(gpvComp.getGlobalParamSet<VXDAlignment>().isConstructed(), false);
    EXPECT_EQ((bool) gpvComp.getGlobalParamSet<VXDAlignment>(), false);

    EXPECT_EQ(gpv.getGlobalParamSet<BeamParameters>().is<BeamParameters>(), true);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamParameters>().is<EmptyGlobalParamSet>(), false);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamParameters>().empty(), false);
    EXPECT_EQ(gpv.getGlobalParamSet<BeamParameters>().isConstructed(), true);
    EXPECT_EQ((bool) gpv.getGlobalParamSet<BeamParameters>(), true);


    EXPECT_EQ(gpvComp.getGlobalParamSet<EmptyGlobalParamSet>().is<EmptyGlobalParamSet>(), true);

    GlobalParamVector newgpv({"BeamParameters", "CDCAlignment"});
    GlobalCalibrationManager::initGlobalVector(newgpv);

    EXPECT_EQ(newgpv.getGlobalParamSet<BeamParameters>().isConstructed(), false);
    newgpv.construct();
    EXPECT_EQ(newgpv.getGlobalParamSet<BeamParameters>().isConstructed(), true);

    newgpv.setGlobalParam(42., BeamParameters::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(BeamParameters::getGlobalUniqueID(), 0, 1), 42.);
    newgpv.loadFromDB(EventMetaData(1, 0, 0));
    EXPECT_EQ(newgpv.getGlobalParam(BeamParameters::getGlobalUniqueID(), 0, 1), 0.);

    newgpv.updateGlobalParam(42., BeamParameters::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(BeamParameters::getGlobalUniqueID(), 0, 1), 42.);

    // CDCAlignment does not contain automatic global params -> set/get does nothing
    // Filled manually by interface from pede result
    newgpv.setGlobalParam(42., CDCAlignment::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(newgpv.getGlobalParam(CDCAlignment::getGlobalUniqueID(), 0, 1), 0.);
  }

}  // namespace
