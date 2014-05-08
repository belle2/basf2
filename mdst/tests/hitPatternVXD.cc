#include <mdst/dataobjects/HitPatternVXD.h>
#include <gtest/gtest.h>

#include <bitset>
#include <string>

namespace Belle2 {
  /** */
  class BitPatternVXDTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(BitPatternVXDTest, settersNGetters)
  {
    unsigned int initValue = 65535;
    std::bitset<32> initValueAsBitset(static_cast<std::string>("00000000000000001111111111111111"));
    HitPatternVXD myHitPattern(initValue);
    EXPECT_EQ(initValue, myHitPattern.getInteger());
    EXPECT_EQ(initValueAsBitset, myHitPattern.getBitset());
  }

  /** Test layer functions. */
  TEST_F(BitPatternVXDTest, layerFunctions)
  {
    unsigned int initValue = 0;
    HitPatternVXD myHitPattern(initValue);

    for (unsigned int layer = 0; layer < 1 /*8*/; ++layer) {
      for (short int nHits = 3; nHits >= 0; --nHits) {
        myHitPattern.setLayer(layer, nHits);
        EXPECT_EQ(true, myHitPattern.hasLayer(layer));
        EXPECT_EQ(nHits, myHitPattern.hitsInLayer(layer));
        --nHits;
        myHitPattern.setLayer(layer, nHits);
        EXPECT_EQ(true, myHitPattern.hasLayer(layer));
        EXPECT_EQ(nHits, myHitPattern.hitsInLayer(layer));
        --nHits;
        myHitPattern.setLayer(layer, nHits);
        EXPECT_EQ(true, myHitPattern.hasLayer(layer));
        EXPECT_EQ(nHits, myHitPattern.hitsInLayer(layer));
        --nHits;
        myHitPattern.setLayer(layer, nHits);
        EXPECT_EQ(false, myHitPattern.hasLayer(layer));
        EXPECT_EQ(nHits, myHitPattern.hitsInLayer(layer));
        unsigned int setForReset = 3;
        myHitPattern.setLayer(layer, setForReset);
        myHitPattern.resetLayer(layer);
        EXPECT_EQ(0, myHitPattern.hitsInLayer(layer));

      }
    }

  }

}
