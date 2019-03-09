#include <svd/dataobjects/SVDModeByte.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

namespace Belle2 {
  namespace SVD {

    /** Check that both constructors yield the same result */
    TEST(SVDModeByte, Constructor)
    {
      // Default construct
      SVDModeByte s0;
      SVDModeByte s_def(151);
      SVDModeByte s_en(SVDRunType::zero_suppressed, SVDEventType::global_run,
                       SVDDAQModeType::daq_6samples, uint8_t(7));
      SVDModeByte s_num(2, 0, 2, 7);

      EXPECT_EQ(s0, s_def);
      EXPECT_EQ(s_def, s_en);
      EXPECT_EQ(s_en, s_num);
      // Empty
      EXPECT_EQ(uint8_t(151), s0.getID());
      EXPECT_EQ((string)s0, "0-suppr/global/6 samples/???");
      // Malformed
      s0.setTriggerBin(3);
      EXPECT_EQ((string)s0, "0-suppr/global/6 samples/3");
      s0.setDAQMode(1);
      EXPECT_EQ((string)s0, "0-suppr/global/3 samples/3");
    }

    /** Check getters and setters */
    TEST(SVDModeByte, getters)
    {
      // Construct
      SVDModeByte s(3, 1, 1, 3);
      EXPECT_EQ(s.getTriggerBin(), 3);
      EXPECT_EQ(s.getDAQMode(), SVDDAQModeType::daq_3samples);
      EXPECT_EQ(s.getEventType(), SVDEventType::local_run);
      EXPECT_EQ(s.getRunType(), SVDRunType::zero_suppressed_timefit);
      // Default
      SVDModeByte s0;
      EXPECT_EQ(s0.getTriggerBin(), 7);
      EXPECT_EQ(s0.getDAQMode(), SVDDAQModeType::daq_6samples);
    }
  }
}  // Belle2::SVD namespace
