#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <framework/logging/Logger.h>

#include <framework/gearbox/Gearbox.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <tracking/cdcLocalTracking/eventdata/collections/CDCWireHitVector.h>


using namespace std;

namespace Belle2 {
  namespace CDCLocalTracking {
    /** Set up a few arrays and objects in the datastore */
    class CDCLocalTrackingTest : public ::testing::Test {
    protected:

      static void SetUpTestCase() {

        Gearbox& gearbox = Gearbox::getInstance();

        vector<string> backends;
        backends.push_back("file:");
        gearbox.setBackends(backends);

        B2INFO("Start open gearbox.");
        gearbox.open("geometry/Belle2.xml");
        B2INFO("Finished open gearbox.");

      }

      static void TearDownTestCase() {

        Gearbox& gearbox = Gearbox::getInstance();
        gearbox.close();
      }

    };

    TEST_F(CDCLocalTrackingTest, WireNeighbors_Symmetry_CWInwards)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      for (const CDCWire & wire : theWireTopology) {
        const CDCWire* neighbor = wire.getNeighborCWInwards();
        if (neighbor != nullptr) {
          const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWOutwards();
          EXPECT_EQ(*neighbor_of_neighbor, wire);
        }
      }
    }

    TEST_F(CDCLocalTrackingTest, WireNeighbors_Symmetry_CCWInwards)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      for (const CDCWire & wire : theWireTopology) {
        const CDCWire* neighbor = wire.getNeighborCCWInwards();
        if (neighbor != nullptr) {
          const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWOutwards();
          EXPECT_EQ(*neighbor_of_neighbor, wire);
        }
      }
    }

    TEST_F(CDCLocalTrackingTest, WireNeighbors_Symmetry_CWOutwards)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      for (const CDCWire & wire : theWireTopology) {
        const CDCWire* neighbor = wire.getNeighborCWOutwards();
        if (neighbor != nullptr) {
          const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWInwards();
          EXPECT_EQ(*neighbor_of_neighbor, wire);
        }
      }
    }

    TEST_F(CDCLocalTrackingTest, WireNeighbors_Symmetry_CCWOutwards)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      for (const CDCWire & wire : theWireTopology) {
        const CDCWire* neighbor = wire.getNeighborCCWOutwards();
        if (neighbor != nullptr) {
          const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWInwards();
          EXPECT_EQ(*neighbor_of_neighbor, wire);
        }
      }
    }

    TEST_F(CDCLocalTrackingTest, WireNeighbors_Symmetry_CCW)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      for (const CDCWire & wire : theWireTopology) {
        const CDCWire* neighbor = wire.getNeighborCCW();
        if (neighbor != nullptr) {
          const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCW();
          EXPECT_EQ(*neighbor_of_neighbor, wire);
        }
      }
    }

    TEST_F(CDCLocalTrackingTest, WireNeighbors_Symmetry_CW)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      for (const CDCWire & wire : theWireTopology) {
        const CDCWire* neighbor = wire.getNeighborCW();
        if (neighbor != nullptr) {
          const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCW();
          EXPECT_EQ(*neighbor_of_neighbor, wire);
        }
      }
    }

    // Tests if CDCGenericHitVector can handle CDCWireHits
    TEST_F(CDCLocalTrackingTest, CDCGenericHitVector_WithCDCWireHits)
    {

      const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

      const CDCWire& firstWire = theWireTopology.first();

      WireID wireID = WireID(firstWire);

      CDCWireHit wirehit(firstWire, 0.0);
      CDCWireHitVector collection;

      ASSERT_TRUE(collection.empty());

      collection.push_back(wirehit);

      ASSERT_FALSE(collection.empty());

    }

  } //end namespace CDCLocalTracking
} //end namespace Belle2

