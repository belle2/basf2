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
#include "../include/Hierarchy.h"



using namespace std;
using namespace Belle2;
using namespace alignment;

namespace Belle2 {
  class PXDSensorAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 21; }
  };
  class PXDLadderAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 22; }
  };
  class PXDLayerAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 23; }
  };
  class PXDAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 24; }
  };

  class FullVXDAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 24; }
  };

  class Belle2Alignment {
  public:
    static unsigned short getGlobalUniqueID() { return 1; }
  };

  class SVDSensorAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 21; }
  };
  class SVDLadderAlignment {
  public:
    static unsigned short getGlobalUniqueID() { return 22; }
  };

  /// Test fixture.
  class HierarchyTest : public ::testing::Test {
  protected:

    /// init
    virtual void SetUp()
    {
    }

    /// cleanup
    virtual void TearDown()
    {

    }

  };

  ///
  TEST_F(HierarchyTest, HierarchyTree)
  {
    auto& hierarchy = HierarchyManager::getInstance().getAlignmentHierarchy();

    VxdID sensorPXD(1, 1, 1);
    VxdID sensorPXD2(1, 1, 2);
    VxdID ladderPXD(1, 1, 0);
    VxdID layerPXD(1, 0, 0);

    VxdID sensorSVD(3, 1, 1);
    VxdID ladderSVD(3, 1, 0);
    VxdID layerSVD(3, 0, 0);

    VxdID pxd(7, 0, 1);
    VxdID svd(7, 0, 2);
    VxdID vxd(7, 1, 1);

    hierarchy.insertG4Transform<PXDSensorAlignment, PXDLadderAlignment>(sensorPXD, ladderPXD, G4Transform3D());
    hierarchy.insertG4Transform<PXDLadderAlignment, PXDLayerAlignment>(ladderPXD, layerPXD, G4Transform3D());
    hierarchy.insertG4Transform<PXDLayerAlignment, PXDAlignment>(layerPXD, pxd, G4Transform3D());
    hierarchy.insertG4Transform<PXDAlignment, Belle2Alignment>(pxd, 0, G4Transform3D());

    hierarchy.insertG4Transform<PXDSensorAlignment, PXDLadderAlignment>(sensorPXD2, ladderPXD, G4Transform3D());


    hierarchy.insertG4Transform<SVDSensorAlignment, SVDLadderAlignment>(sensorSVD, ladderSVD, G4Transform3D());
    hierarchy.insertG4Transform<SVDLadderAlignment, Belle2Alignment>(ladderSVD, 0, G4Transform3D());

    // hierarchyPXD.printHierarchy();
    // hierarchySVD.printHierarchy();

    auto sop = new genfit::StateOnPlane(nullptr);
    TVectorD state(5);
    state[0] = 0.;
    state[1] = 0.;
    state[2] = 0.;
    state[3] = 0.;
    state[4] = 0.;
    sop->setState(state);
    hierarchy.getGlobalDerivatives<PXDSensorAlignment>(sensorPXD, sop).second.Print();
    Constraints constr;
    hierarchy.buildConstraints(constr);

    std::map<std::string, int> Map;
    for (unsigned int i = 0; i < 200000; ++i) {
      Map.insert({std::to_string(i), i});
    }
    for (unsigned int i = 0; i < 200000; ++i) {
      Map.find(std::to_string(i));
    }
  }






}  // namespace
