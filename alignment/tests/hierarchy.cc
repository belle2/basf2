
#include <alignment/Hierarchy.h>
#include <alignment/Manager.h>
#include <alignment/PedeResult.h>
#include <framework/database/EventDependency.h>
#include <vxd/dataobjects/VxdID.h>

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
using namespace Belle2;
using namespace alignment;

namespace {
  /// Test class for hierarchy parameter storage
  class TestTopLevelDBObj {
  public:
    static unsigned short getGlobalUniqueID() { return 10; }
  };

  /// Test class for hierarchy parameter storage
  class TestLevelDBObj {
  public:
    static unsigned short getGlobalUniqueID() { return 20; }
  };

  /// Test fixture.for hierarchy
  class HierarchyTest : public ::testing::Test {
  protected:
    unsigned short topElement {1};
    unsigned short element1 = {1};
    unsigned short element2 = {2};

    /// init - fill hierarchy
    virtual void SetUp()
    {
      auto& hierarchy = GlobalCalibrationManager::getInstance().getAlignmentHierarchy();

      G4Transform3D unitTrafo;
      hierarchy.insertG4Transform<TestLevelDBObj, TestTopLevelDBObj>(element1, topElement, unitTrafo);
      hierarchy.insertG4Transform<TestLevelDBObj, TestTopLevelDBObj>(element2, topElement, unitTrafo);
    }

    /// cleanup
    virtual void TearDown()
    {

    }

  };

  /// Test calculation of global derivatives
  TEST_F(HierarchyTest, HierarchyDerivatives)
  {
    auto& hierarchy = GlobalCalibrationManager::getInstance().getAlignmentHierarchy();

    auto sop = new genfit::StateOnPlane(nullptr);
    TVectorD state(5);
    state[0] = 1.;
    state[1] = 1.;
    state[2] = 1.;
    state[3] = 1.;
    state[4] = 1.;
    sop->setState(state);

    // The du/dU derivative should be one everywhere (when we move along X, we incerase residual in X by same amount)
    EXPECT_EQ(hierarchy.getGlobalDerivatives<TestLevelDBObj>(element1, sop).second(0, 0), 1.);

  }

  /// Test constraint generation
  TEST_F(HierarchyTest, HierarchyConstraints)
  {
    auto& hierarchy = GlobalCalibrationManager::getInstance().getAlignmentHierarchy();

    Constraints constraints;
    hierarchy.buildConstraints(constraints);

    // We should have 6 constraints - one per each top level parameter
    EXPECT_EQ(constraints.size(), 6);

    // Here each child parameter exactly corresponds to mother parameter
    // we have two childs, so there should be two entries in constraint coefficients
    for (auto checksum_constraint : constraints) {
      auto& constraint = checksum_constraint.second;
      EXPECT_EQ(constraint.size(), 2);
      // ... and the coefficients should be just ones
      EXPECT_EQ(fabs(constraint[0].second), 1.);
    }

    //HierarchyManager::getInstance().writeConstraints("constraints.txt");

  }

}  // namespace
