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
#include <TMath.h>



using namespace std;
using namespace Belle2;
using namespace alignment;

namespace {
  /// Test class for hierarchy parameter storage
  class MockDBObj : public TObject {
  public:
    static unsigned short getGlobalUniqueID() { return 1; }
    double getGlobalParam(unsigned short, unsigned short) {return m_value;}
    void setGlobalParam(unsigned short, unsigned short, double val) {m_value = val;}
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {{0, 1}};}
  private:
    double m_value {0.};
  };

  class MockDetectorInterface : public IGlobalParamInterface {

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

  /// Test calculation of global derivatives
  TEST_F(GlobalParamTest, GlobalParamComponentsInterfaces)
  {
    GlobalParamVector gpvFull;
    GlobalParamVector gpv({"MockDBObj"});

    std::shared_ptr<IGlobalParamInterface> interface = std::shared_ptr<IGlobalParamInterface>(new MockDetectorInterface);

    gpvFull.addDBObj<MockDBObj>(interface);
    gpv.addDBObj<MockDBObj>(interface);

    gpv.setGlobalParam(42., MockDBObj::getGlobalUniqueID(), 0, 1);
    EXPECT_EQ(gpv.getGlobalParam(MockDBObj::getGlobalUniqueID(), 0, 1), 42.);

  }

}  // namespace
