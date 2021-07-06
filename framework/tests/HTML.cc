/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
