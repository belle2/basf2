/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/RingBuffer.h>

#include <gtest/gtest.h>

#include <memory>
#include <numeric>
#include <vector>

using namespace Belle2;

namespace {

  /** Fixture providing a small private RingBuffer. */
  class RingBufferCapacityTest : public ::testing::Test {
  protected:
    /** A few thousand words is plenty and keeps the shared memory segment small. */
    static const int c_sizeWords = 4096;

    /** Build a record of nWords filled with a recognisable pattern. */
    static std::vector<int> makeRecord(int nWords, int seed)
    {
      std::vector<int> record(nWords);
      std::iota(record.begin(), record.end(), seed);
      return record;
    }

    std::unique_ptr<RingBuffer> m_rb{new RingBuffer(c_sizeWords)};   /**< buffer under test */
  };

  /** An empty buffer returns 0 whatever capacity is offered. */
  TEST_F(RingBufferCapacityTest, EmptyReturnsZero)
  {
    std::vector<int> buf(16);
    EXPECT_EQ(0, m_rb->remq(buf.data(), buf.size()));
    EXPECT_EQ(0, m_rb->remq(buf.data(), 0));
    EXPECT_EQ(0, m_rb->remq(buf.data()));
  }

  /** A capacity that fits returns the size in words and the payload. */
  TEST_F(RingBufferCapacityTest, FittingCapacityCopies)
  {
    const std::vector<int> record = makeRecord(32, 1000);
    ASSERT_GT(m_rb->insq(record.data(), record.size()), 0);

    std::vector<int> buf(64);
    const int size = m_rb->remq(buf.data(), buf.size());
    ASSERT_EQ(static_cast<int>(record.size()), size);
    EXPECT_TRUE(std::equal(record.begin(), record.end(), buf.begin()));

    // and it really was consumed
    EXPECT_EQ(0, m_rb->remq(buf.data(), buf.size()));
  }

  /** The central contract: too small reports the need and consumes nothing. */
  TEST_F(RingBufferCapacityTest, TooSmallReportsSizeAndDoesNotDequeue)
  {
    const std::vector<int> record = makeRecord(100, 7);
    ASSERT_GT(m_rb->insq(record.data(), record.size()), 0);
    ASSERT_EQ(1, m_rb->numq());

    // rejecting is idempotent: the record survives every attempt
    std::vector<int> small(10);
    int needed = 0;
    for (int attempt = 0; attempt < 3; ++attempt) {
      needed = m_rb->remq(small.data(), small.size());
      EXPECT_EQ(-static_cast<int>(record.size()), needed)
          << "remq must return minus the required size in words, attempt " << attempt;
      EXPECT_EQ(1, m_rb->numq())
          << "the record must still be queued so the caller can retry, attempt " << attempt;
    }

    // Exactly the reported size must be enough, with nothing to spare.
    std::vector<int> exact(-needed);
    const int size = m_rb->remq(exact.data(), exact.size());
    ASSERT_EQ(static_cast<int>(record.size()), size);
    EXPECT_TRUE(std::equal(record.begin(), record.end(), exact.begin()));
    EXPECT_EQ(0, m_rb->numq());
  }

  /** A rejected record does not block the ones behind it once we grow. */
  TEST_F(RingBufferCapacityTest, GrowingRecoversEveryRecordInOrder)
  {
    const std::vector<int> first = makeRecord(20, 100);
    const std::vector<int> second = makeRecord(200, 500);
    ASSERT_GT(m_rb->insq(first.data(), first.size()), 0);
    ASSERT_GT(m_rb->insq(second.data(), second.size()), 0);

    // RxModule's loop: start small, grow on demand, never lose a record.
    std::vector<int> buf(4);
    std::vector<std::vector<int>> got;
    while (got.size() < 2) {
      const int size = m_rb->remq(buf.data(), buf.size());
      if (size > 0) {
        got.emplace_back(buf.begin(), buf.begin() + size);
      } else {
        ASSERT_LT(size, 0) << "buffer unexpectedly empty";
        buf.resize(-size);
      }
    }
    EXPECT_EQ(first, got[0]);
    EXPECT_EQ(second, got[1]);
  }

  /** Back compatibility: capacityWords defaults to -1, meaning "do not check". */
  TEST_F(RingBufferCapacityTest, UncheckedCallStillWorks)
  {
    const std::vector<int> record = makeRecord(30, 11);
    ASSERT_GT(m_rb->insq(record.data(), record.size()), 0);

    std::vector<int> buf(c_sizeWords);
    EXPECT_EQ(static_cast<int>(record.size()), m_rb->remq(buf.data()));
    EXPECT_TRUE(std::equal(record.begin(), record.end(), buf.begin()));
  }

  /** TxModule discards with remq(nullptr); a capacity must not change that. */
  TEST_F(RingBufferCapacityTest, NullBufferDiscardsRegardlessOfCapacity)
  {
    const std::vector<int> record = makeRecord(80, 13);
    ASSERT_GT(m_rb->insq(record.data(), record.size()), 0);

    EXPECT_EQ(static_cast<int>(record.size()), m_rb->remq(nullptr, 1));
    EXPECT_EQ(0, m_rb->numq()) << "a null buffer must still consume the record";
  }

}  // namespace
