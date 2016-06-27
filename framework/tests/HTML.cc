#include <framework/utilities/HTML.h>


#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(Utilities, HTML)
  {
    std::string s("\"abc<def>geh &abc;&gt;");
    EXPECT_EQ(s, HTML::unescape(HTML::escape(s)));

    EXPECT_EQ("&quot;abc&lt;def&gt;geh &amp;abc;&amp;gt;", HTML::escape(s));
  }
}
