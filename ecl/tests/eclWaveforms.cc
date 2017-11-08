#include <ecl/dataobjects/ECLWaveforms.h>
#include <ecl/digitization/ECLCompress.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Declare ECLWaveforms test */
  class ECLWaveformsTest : public ::testing::Test {};

  /** Test Constructor. */
  TEST_F(ECLWaveformsTest, Constructor)
  {
    ECLWaveforms q;
    EXPECT_EQ(q.getStore().size(), 0);
  }

  /** Test the only method. */
  TEST_F(ECLWaveformsTest, Getter)
  {
    const int N = 2;
    vector<unsigned int> t(N);
    ECLWaveforms q;
    t.swap(q.getStore());
    EXPECT_EQ(q.getStore().size(), N);
  }

  /** Declare BitStream test */
  class BitStreamTest : public ::testing::Test {};

  /** Test Constructor. */
  TEST_F(BitStreamTest, Constructor)
  {
    BitStream q;
    EXPECT_EQ(q.getPos(), 0);
    EXPECT_EQ(q.getStore().size(), 0);

    int N = 128;
    BitStream q2(N);
    EXPECT_EQ(q2.getPos(), 0);
    EXPECT_EQ(q2.getStore().size(), N);
  }

  /** Test class functionality */
  TEST_F(BitStreamTest, functionality)
  {
    int N = 128;
    BitStream q(N);

    for (int i = 0; i < 32; i++) {
      unsigned int k = 1 << i;
      q.putNBits(k, i + 1);
    }
    EXPECT_EQ(q.getPos(), 528);
    q.setPos(0);
    for (int i = 0; i < 32; i++) {
      unsigned int k0 = 1 << i, k = q.getNBits(i + 1);
      EXPECT_EQ(k, k0);
    }
    EXPECT_EQ(q.getPos(), 528);

    q.resize();
    q.setPos(0);
    for (int i = 0; i < 32; i++) {
      unsigned int k0 = 1 << i, k = q.getNBits(i + 1);
      EXPECT_EQ(k, k0);
    }
    EXPECT_EQ(q.getPos(), 528);
  }

  /** Declare ECLBaseCompress test */
  class ECLBaseCompressTest : public ::testing::Test {};

  /** Test class functionality */
  TEST_F(ECLBaseCompressTest, functionality)
  {
    ECLBaseCompress q;

    int N = 128;
    for (int off = 0; off < 32; ++off) {
      BitStream q2(N);

      q2.setPos(off);
      int A[31];
      for (int i = 0; i < 31; i++) A[i] = 4000 + ((i % 2) * 1000 - 500);
      q.compress(q2, A);

      q2.setPos(off);
      int B[31];
      q.uncompress(q2, B);
      for (int i = 0; i < 31; i++) EXPECT_EQ(A[i], B[i]);
    }
  }

  /** Declare ECLDeltaCompress test */
  class ECLDeltaCompressTest : public ::testing::Test {};

  /** Test class functionality */
  TEST_F(ECLDeltaCompressTest, functionality)
  {
    ECLDeltaCompress q;

    int N = 128;
    for (int off = 0; off < 32; ++off) {
      BitStream q2(N);

      q2.setPos(off);
      int A[31];
      for (int i = 0; i < 31; i++) A[i] = 4000 + ((i % 2) * 1000 - 500);
      q.compress(q2, A);

      q2.setPos(off);
      int B[31];
      q.uncompress(q2, B);
      for (int i = 0; i < 31; i++) EXPECT_EQ(A[i], B[i]);
    }
  }

  /** Declare ECLDCTCompress test */
  class ECLDCTCompressTest : public ::testing::Test {};

  /** Test class functionality */
  TEST_F(ECLDCTCompressTest, functionality)
  {
    width_t widths_phs2_scale10[] = {
      {5, 7, 9, 32},// 5.82104
      {4, 6, 8, 32},// 4.76806
      {4, 6, 8, 32},// 4.70815
      {4, 6, 8, 32},// 4.61517
      {3, 5, 7, 32},// 4.42656
      {3, 5, 7, 32},// 4.22157
      {3, 5, 7, 32},// 4.01412
      {3, 5, 7, 32},// 3.80959
      {2, 4, 6, 32},// 3.60224
      {2, 4, 6, 32},// 3.31705
      {2, 4, 6, 32},// 3.03457
      {2, 3, 5, 32},// 2.71501
      {2, 3, 5, 32},// 2.45094
      {2, 3, 5, 32},// 2.25788
      {2, 3, 5, 32},// 2.13303
      {2, 3, 5, 32},// 2.06428
      {2, 3, 5, 32},// 2.02847
      {2, 3, 5, 32},// 2.01253
      {1, 2, 4, 32},// 1.86085
      {1, 2, 4, 32},// 1.68465
      {1, 2, 4, 32},// 1.53003
      {1, 2, 4, 32},// 1.38031
      {1, 2, 4, 32},// 1.27103
      {1, 2, 4, 32},// 1.18264
      {1, 2, 4, 32},// 1.11546
      {1, 2, 4, 32},// 1.07223
      {1, 2, 4, 32},// 1.04641
      {1, 2, 4, 32},// 1.03003
      {1, 2, 4, 32},// 1.01772
      {1, 2, 3, 32},// 1.01304
      {1, 2, 4, 32},// 1.0107
    };

    ECLDCTCompress q(1, 3500, widths_phs2_scale10);

    int N = 128;
    for (int off = 0; off < 1; ++off) {
      BitStream q2(N);

      q2.setPos(off);
      int A[31];
      for (int i = 0; i < 31; i++) A[i] = 4000 + ((i % 2) * 1000 - 500);
      q.compress(q2, A);

      q2.setPos(off);
      int B[31];
      q.uncompress(q2, B);
      for (int i = 0; i < 31; i++) EXPECT_NEAR(A[i], B[i], 10);
    }
  }
}  // namespace
