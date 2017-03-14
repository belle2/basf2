/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <framework/utilities/IOIntercept.h>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** test that capturing stdout works */
  TEST(IOIntercept, CaptureStdOut)
  {
    IOIntercept::CaptureStdOut capture;
    ASSERT_FALSE(capture.finish());
    ASSERT_TRUE(capture.start());
    ASSERT_TRUE(capture.start());
    std::cout << "this is a test";
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "this is a test");
    ASSERT_FALSE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "this is a test");

    // Ok, again with raw write to fd
    ASSERT_TRUE(capture.start());
    write(fileno(stdout), "this is a test", 14);
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "this is a test");
  }
  /** test that capturing stderr works */
  TEST(IOIntercept, CaptureStdErr)
  {
    IOIntercept::CaptureStdErr capture;
    ASSERT_FALSE(capture.finish());
    ASSERT_TRUE(capture.start());
    ASSERT_TRUE(capture.start());
    std::cerr << "this is a test";
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdErr(), "this is a test");
    ASSERT_FALSE(capture.finish());
    ASSERT_EQ(capture.getStdErr(), "this is a test");

    // Ok, again with raw write to fd
    ASSERT_TRUE(capture.start());
    write(fileno(stderr), "this is a test", 14);
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdErr(), "this is a test");

  }

  /** test if capturing large output works as expected */
  TEST(IOIntercept, CaptureLargeOutput)
  {
    IOIntercept::CaptureStdOut capture;
    ASSERT_TRUE(capture.start());
    std::string out;
    int written{0};
    for (int i = 0; i < 100000; ++i) {
      std::cout << (char)(i % 255) << flush;
      if (std::cout.good()) written = i + 1;
      out.push_back(i % 255);
    }
    ASSERT_TRUE(capture.finish());
    const std::string& captured = capture.getStdOut();
    // pipes have a limited capacity. We create them non-blocking which
    // means that once they're full we will just not get any output back.
    // So check that the begin of the capture works
    ASSERT_TRUE(captured.size() <= out.size());
    ASSERT_EQ(captured.size(), written);
    if (captured.size() < out.size()) {
      std::cout << "Output truncated after " << captured.size() << " bytes" << std::endl;
    }
    ASSERT_EQ(captured, out.substr(0, captured.size()));
    // and that capturing still works after overflow
    ASSERT_TRUE(capture.start());
    std::cout << "once more";
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "once more");

    // and once more for C
    ASSERT_TRUE(capture.start());
    printf("once more");
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "once more");
  }

  /** this function generates a bit of output on stderr and exits. If
   * discarding output works the output is "start-><-end", otherwise there
   * will be additional text between the arrows */
  void generateStdErr()
  {
    IOIntercept::DiscardStdErr discard;
    std::cerr << "start->";
    discard.start();
    std::cerr << "this should not show up" << std::endl << std::flush;
    write(fileno(stderr), "nor this\n", 9);
    discard.finish();
    std::cerr << "<-end";
    std::exit(0);
  }

  /** test discarding output */
  TEST(IOInterceptDeathTest, DiscardStdOut)
  {
    IOIntercept::DiscardStdOutStdErr discard;
    ASSERT_FALSE(discard.finish());
    ASSERT_TRUE(discard.start());
    ASSERT_TRUE(discard.start());
    ASSERT_TRUE(discard.finish());
    ASSERT_FALSE(discard.finish());
    // hard to test if there's no output ... let's use a death test and
    // verify that stderr of child process matches what we expect
    EXPECT_EXIT(generateStdErr(), ::testing::ExitedWithCode(0), "^start-><-end$");
  }
}
