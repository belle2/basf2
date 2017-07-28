#include <svd/dataobjects/SVDModeByte.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

namespace Belle2 {

  /** Check that both constructors yield the same result */
  TEST(SVDModeByte, Constructor)
  {
    // Construct
    SVDModeByte s(3, SVDDAQModeType::zerosupp_6samples);
    SVDModeByte l(30); // 30 = 011 110
    EXPECT_EQ(l, s);
    EXPECT_EQ((string)l, "trg 3 DAQ 0-supp 6 samples");
    // Empty
    SVDModeByte s0; // id = 62, 111 110, trg 7, 6 samples
    EXPECT_EQ(62, s0.getID());
    EXPECT_EQ((string)s0, "trg * DAQ 0-supp 6 samples");
    // Malformed
    l.setTriggerBin(5);
    EXPECT_EQ((string)l, "trg * DAQ 0-supp 6 samples");
    l.setTriggerBin(3);
    l.setDAQMode(static_cast<SVDDAQModeType>(4));
    EXPECT_EQ((string)l, "trg 3 DAQ unknown mode");
  }
  /** Check getters and setters */
  TEST(SVDModeByte, getters)
  {
    // Construct
    SVDModeByte s(3, SVDDAQModeType::zerosupp_6samples);
    EXPECT_EQ(s.getTriggerBin(), 3);
    EXPECT_EQ(s.getTriggerInterval().first, -31.44 * 0.25);
    EXPECT_EQ(s.getTriggerInterval().second, 0);
    EXPECT_EQ(s.getDAQMode(), SVDDAQModeType::zerosupp_6samples);
    // Default
    SVDModeByte s0;
    EXPECT_EQ(s0.getTriggerBin(), 7);
    EXPECT_EQ(s0.getTriggerInterval().first, -31.44);
    EXPECT_EQ(s0.getTriggerInterval().second, 0.0);
    EXPECT_EQ(s0.getDAQMode(), SVDDAQModeType::zerosupp_6samples);
  }
}  // namespace
