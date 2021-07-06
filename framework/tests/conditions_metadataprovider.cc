/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/MetadataProvider.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/testhelpers/Fixtures.h>
#include <gtest/gtest.h>

using namespace Belle2;
using namespace Conditions;

namespace {
  using PayloadMetadata = PayloadMetadata;
  /** Dummy Metadata provider for testing: given a list of payloads on construction return those when requested */
  class TestMetadataProvider final: public MetadataProvider {
    /** payloads available */
    std::vector<PayloadMetadata> m_testpayloads;
  public:
    /** Create with a list of payloads */
    explicit TestMetadataProvider(const std::vector<PayloadMetadata>& payloads): m_testpayloads(payloads) {}
    /** Check global tag status: basically just return either PUBLISHED if the
     * name starts with "tag" or the name of the tag itself otherwise to be able
     * to test rejection of tag states */
    std::string getGlobaltagStatus(const std::string& name) override
    {
      if (name.empty()) {
        B2ERROR("This globaltag is missing" << LogVar("globaltag", "") << LogVar("error", "missing"));
      }
      // later we want to test payloads from different tags so if the tagname starts with "tag" assume valid;
      if (name.substr(0, 3) == "tag") {
        return "PUBLISHED";
      }
      // otherwise just treat the tag name as its status to allow testing tag states
      return name;
    }
    /** Called by the base class when payloads are requested. Just look through
     * the list of predefined payloads and add the ones in the correct globaltag
     * and exp/run range. */
    bool updatePayloads(const std::string& globaltag, int exp, int run) override
    {
      IntervalOfValidity iov(exp, run, exp, run);
      int count{0};
      for (auto p : m_testpayloads) {
        if (p.globaltag == globaltag and p.iov.contains(iov)) {
          addPayload(std::move(p), "testprovider");
          ++count;
        }
      }
      return count > 0;
    }
  };

  /** We want to inspect the log messages emitted */
  using MetadataProviderTest = TestHelpers::LogMessageTest;

  /** Check whether states get rejected correctly */
  TEST_F(MetadataProviderTest, tagstates)
  {
    TestMetadataProvider provider({});
    EXPECT_TRUE(provider.setTags({"TESTING", "VALIDATED", "RUNNING", "PUBLISHED"}));
    EXPECT_FALSE(provider.setTags({"OPEN"}));
    // this will also conveniently fail if the first expect did produce an error
    expectErrorWithVariables({{"globaltag", "OPEN"}, {"status", "OPEN"}});
    EXPECT_FALSE(provider.setTags({"INVALID"}));
    expectErrorWithVariables({{"globaltag", "INVALID"}, {"status", "INVALID"}});
    // also fail if the OPEN one is somewhere in the middle
    EXPECT_FALSE(provider.setTags({"TESTING", "VALIDATED", "OPEN", "RUNNING", "PUBLISHED"}));
    expectErrorWithVariables({{"globaltag", "OPEN"}, {"status", "OPEN"}});
    provider.setUsableTagStates({"OPEN", "INVALID"});
    EXPECT_TRUE(provider.setTags({"OPEN"}));
    EXPECT_FALSE(provider.setTags({"INVALID"}));
    expectErrorWithVariables({{"globaltag", "INVALID"}, {"status", "INVALID"}});
    EXPECT_FALSE(provider.setTags({""}));
    expectErrorWithVariables({{"globaltag", ""}, {"error", "missing"}});
  }

  /** Test that returning false from updatePayloads throws an exception */
  TEST_F(MetadataProviderTest, exception)
  {
    TestMetadataProvider provider({});
    std::vector<PayloadMetadata> query{PayloadMetadata{"A"}};
    ASSERT_TRUE(provider.setTags({"tag1"}));
    ASSERT_THROW(provider.getPayloads(0, 0, query), std::runtime_error);
  }

  /** Test that the metadata provider only fills metadata for payloads which
   * don't already have revision information */
  TEST_F(MetadataProviderTest, onlyfillneeded)
  {
    TestMetadataProvider provider({
      //name, tag, ignore, ignore, ignore, exp, run, exp, run, revision
      PayloadMetadata{"A", "tag1", "", "", "", 0, 0, -1, -1, 2},
    });
    std::vector<PayloadMetadata> query{PayloadMetadata{"A"}, PayloadMetadata{"B"}};
    // A is in the tag but already set. B isn't in the tag but already set so no error
    query[0].revision = 1;
    query[1].revision = 1;
    ASSERT_TRUE(provider.setTags({"tag1"}));
    ASSERT_TRUE(provider.getPayloads(0, 0, query));
    // so no log messages please
    expectMessage(LogConfig::c_Error, 0, true);
    // and everything as it was
    EXPECT_EQ(query[0].name, "A");
    EXPECT_EQ(query[0].revision, 1);
    EXPECT_EQ(query[1].name, "B");
    EXPECT_EQ(query[1].revision, 1);
  }

  /** Test obtaining payloads from a single globaltag */
  TEST_F(MetadataProviderTest, payloads)
  {
    TestMetadataProvider provider({
      //name, tag, ignore, ignore, ignore, exp, run, exp, run, revision
      PayloadMetadata{"A", "tag1", "", "", "", 0, 0, -1, -1, 1},
      PayloadMetadata{"A", "tag1", "", "", "", 1, 0, 1, 10, 3},
      PayloadMetadata{"A", "tag1", "", "", "", 1, 0, 1, 10, 2},
      PayloadMetadata{"B", "tag1", "", "", "", 1, 0, 1, 10, 1},
    });
    std::vector<PayloadMetadata> query{PayloadMetadata{"A"}, PayloadMetadata{"B"}};
    ASSERT_TRUE(provider.setTags({"tag1"}));
    ASSERT_FALSE(provider.getPayloads(0, 0, query));
    expectErrorWithVariables({{"globaltags", "tag1"}, {"name", "B"}, {"experiment", "0"}, {"run", "0"}});
    EXPECT_EQ(query[0].revision, 1);
    EXPECT_EQ(query[1].revision, 0);
    // try for exp 1, there should be something for both payloads and A should be in revision 3.
    // Howewer metadata provider only fills missing info so we need new query structure
    query = {PayloadMetadata{"A"}, PayloadMetadata{"B"}};
    EXPECT_TRUE(provider.getPayloads(1, 0, query));
    EXPECT_EQ(query[0].revision, 3);
    EXPECT_EQ(query[1].revision, 1);
    // and back to exp 0 and we want the same result as before.
    query = {PayloadMetadata{"A"}, PayloadMetadata{"B"}};
    EXPECT_FALSE(provider.getPayloads(0, 0, query));
    expectErrorWithVariables({{"globaltags", "tag1"}, {"name", "B"}, {"experiment", "0"}, {"run", "0"}});
    EXPECT_EQ(query[0].revision, 1);
    EXPECT_EQ(query[1].revision, 0);
    // so let's try this again but mark B as optional. Same result but no error
    query = {PayloadMetadata{"A"}, PayloadMetadata{"B", false}};
    EXPECT_TRUE(provider.getPayloads(0, 0, query));
    EXPECT_EQ(query[0].revision, 1);
    EXPECT_EQ(query[1].revision, 0);
    EXPECT_EQ(query[1].required, false);
  }

  TEST_F(MetadataProviderTest, payloads_multiple_gt)
  {
    TestMetadataProvider provider({
      //name, tag, ignore, ignore, ignore, exp, run, exp, run, revision
      PayloadMetadata{"A", "tag1", "", "", "", 0, 0, -1, -1, 1},
      PayloadMetadata{"A", "tag2", "", "", "", 0, 0, -1, -1, 2},
      PayloadMetadata{"B", "tag2", "", "", "", 0, 0, -1, -1, 2},
    });
    // In this case A should be taken from tag1 even if it exists in tag2.
    // But B can only come from tag2
    ASSERT_TRUE(provider.setTags({"tag1", "tag2"}));
    std::vector<PayloadMetadata> query = {PayloadMetadata{"A"}, PayloadMetadata{"B"}};
    EXPECT_TRUE(provider.getPayloads(1, 0, query));
    EXPECT_EQ(query[0].revision, 1);
    EXPECT_EQ(query[1].revision, 2);
    EXPECT_EQ(query[0].globaltag, "tag1");
    EXPECT_EQ(query[1].globaltag, "tag2");
  }
}
