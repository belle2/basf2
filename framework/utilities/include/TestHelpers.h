/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/LogSystem.h>

#include <gtest/gtest.h>

#include <string>

namespace Belle2 {
  /** Some utilities to help with writing unit tests.
   *
   * It is possible to nest these macros with others to test two things, e.g.:
   * \code
     EXPECT_B2ERROR(EXPECT_DOUBLE_EQ(5e3, Unit::convertValue(5e3, "nonexistingunit")));
     \endcode
   */
  namespace TestHelpers {

    /** This class provides a test fixture managing the opening and closing
     *  of the Gearbox with the default geometry file.
     *
     * Use like
     \code
     TEST_F(TestWithGearBox, MyTest)
     {

     }
     \endcode
     *
     */
    class TestWithGearbox : public ::testing::Test {

    public:
      /// Sets up the Gearbox once for all test in this TestCase.
      static void SetUpTestCase();

      /// Closes the Gearbox once for all test in this TestCase.
      static void TearDownTestCase();
    };

    /** changes working directory into a newly created directory, and removes it (and contents) on destruction.
     *
     * Example:
     *
      \code
      TEST(MyTest, TestThatCreatesFiles)
      {
        TestHelpers::TempDirCreator tempdir;
        //create files here ... (automatically cleaned up at end of scope)
      }
      \endcode
     */
    class TempDirCreator {
    public:
      TempDirCreator(); /**< ctor. */
      ~TempDirCreator(); /**< dtor. */

      /**
       * Returns path of temporary directory
       */
      std::string getTempDir() const;
    private:
      std::string m_oldpwd; /**< previous working directory. */
      std::string m_tmpdir; /**< path of temporary director. */
    };
  }
}

/** \def EXPECT_LOGMESSAGE(x, loglevel)
 *
 *  Command x should print at least one message of given loglevel using basf2's logging system.
 *  \sa EXPECT_B2FATAL
 *  \sa EXPECT_B2ERROR
 *  \sa EXPECT_B2WARNING
 */
#define EXPECT_LOGMESSAGE(x, loglevel) \
  do { \
    int nmessages_before = Belle2::LogSystem::Instance().getMessageCounter(loglevel); \
    { \
      x; \
    } \
    int nmessages_after = Belle2::LogSystem::Instance().getMessageCounter(loglevel); \
    EXPECT_TRUE(nmessages_after > nmessages_before) << "Message of level " << #loglevel << " expected, but not found."; \
  } while(0)

/** \def EXPECT_B2FATAL(x)
 *
 * command x should exit using B2FATAL.
 * Note than due to a bug in root versions before 6, this will also pass if x causes a segmentation fault or similar crash. Please check manually that you do get a B2FATAL message.
 */
#define EXPECT_B2FATAL(x) EXPECT_EXIT(x, ::testing::ExitedWithCode(1),"")

/** \def EXPECT_B2ERROR(x)
 *
 *  command x should print at least one B2ERROR.
 */
#define EXPECT_B2ERROR(x) EXPECT_LOGMESSAGE(x, Belle2::LogConfig::c_Error)

/** \def EXPECT_B2WARNING(x)
 *
 *  command x should print at least one B2WARNING.
 */
#define EXPECT_B2WARNING(x) EXPECT_LOGMESSAGE(x, Belle2::LogConfig::c_Warning)

/** \def EXPECT_NO_LOGMESSAGE(x, loglevel)
 *
 *  Command x should not print any message of given loglevel using basf2's logging system.
 *  \sa EXPECT_NO_B2FATAL
 *  \sa EXPECT_NO_B2ERROR
 *  \sa EXPECT_NO_B2WARNING
 */
#define EXPECT_NO_LOGMESSAGE(x, loglevel) \
  do { \
    int nmessages_before = Belle2::LogSystem::Instance().getMessageCounter(loglevel); \
    { \
      x; \
    } \
    int nmessages_after = Belle2::LogSystem::Instance().getMessageCounter(loglevel); \
    EXPECT_TRUE(nmessages_after == nmessages_before) << "Message of level " << #loglevel << " found, but non expected."; \
  } while(0)

/** \def EXPECT_NO_B2FATAL(x)
 *
 *  command x should print no B2FATAL.
 */
#define EXPECT_NO_B2FATAL(x) EXPECT_NO_LOGMESSAGE(x, Belle2::LogConfig::c_Fatal)

/** \def EXPECT_NO_B2ERROR(x)
 *
 *  command x should print no B2ERROR.
 */
#define EXPECT_NO_B2ERROR(x) EXPECT_NO_LOGMESSAGE(x, Belle2::LogConfig::c_Error)

/** \def EXPECT_NO_B2WARNING(x)
 *
 *  command x should print no B2WARNING.
 */
#define EXPECT_NO_B2WARNING(x) EXPECT_NO_LOGMESSAGE(x, Belle2::LogConfig::c_Warning)

/** \def TEST_CONTEXT(x)
 *
 *  Adds a message to all EXCEPTS and ASSERT in the current scope and any called or nested scopes
 *
 *  The macro sets up for the following EXCEPTS and ASSERTS, hence it must be placed before the tests.
 *
 *  The message can be composed in a B2INFO style manner with addtional << between
 *  individual strings and values to be concatenated.
 *
 *  \code TEST_CONTEXT("for my value set to "  << myValue); \endcode
 */
#define TEST_CONTEXT(message) SCOPED_TRACE([&](){std::ostringstream messageStream; messageStream << message; return messageStream.str();}())


/** \def EXPECT_ANGLE_NEAR(expected, actual, tolerance)
 *  Expectation macro for angle values that should not care for a multiple of 2 * PI difference between the values
 */
#define EXPECT_ANGLE_NEAR(expected, actual, tolerance) EXPECT_PRED3(::Belle2::TestHelpers::angleNear, expected, actual, tolerance)

/** \def ASSERT_ANGLE_NEAR(expected, actual, tolerance)
 *  Assertation macro for angle values that should not care for a multiple of 2 * PI difference between the values
 */
#define ASSERT_ANGLE_NEAR(expected, actual, tolerance) ASSERT_PRED3(::Belle2::TestHelpers::angleNear, expected, actual, tolerance)

/** \def EXPECT_SAME_SIGN(expected, actual)
 *  Expectation macro that two values carry the same sign.
 */
#define EXPECT_SAME_SIGN(expected, actual) EXPECT_PRED2(::Belle2::TestHelpers::sameSign, expected, actual)

/** \def ASSERT_SAME_SIGN(expected, actual)
 * Assertation macro that two values carry the same sign.
 */
#define ASSERT_SAME_SIGN(expected, actual) ASSERT_PRED2(::Belle2::TestHelpers::sameSign, expected, actual)


/** \def EXPECT_POSITIVE(expected, actual)
 *  Expectation macro that a value is bigger than zero.
 */
#define EXPECT_POSITIVE(expected) EXPECT_PRED1(::Belle2::TestHelpers::isPositive, expected)

/** \def ASSERT_POSITIVE(expected, actual)
 *  Assertation macro that a value is bigger than zero.
 */
#define ASSERT_POSITIVE(expected) ASSERT_PRED1(::Belle2::TestHelpers::isPositive, expected)

/** \def EXPECT_NEGATIVE(expected, actual)
 *   Expectation macro that a value is smaller than zero.
 */
#define EXPECT_NEGATIVE(expected) EXPECT_PRED1(::Belle2::TestHelpers::isNegative, expected)

/** \def ASSERT_NEGATIVE(expected, actual)
 *  Assertation macro that a value is smaller than zero.
 */
#define ASSERT_NEGATIVE(expected) ASSERT_PRED1(::Belle2::TestHelpers::isNegative, expected)

/**
 * Expectation macro for combound structures containing floating point values like TVector3, etc. to be close to each other.
 *
 * Generally it compares each contained floating point value to be no less than the tolerance apart.
 *
 * The macro uses the templated predicate template<class T> allNear(const T& expected, const T& actual, double tolerance)
 * to compute whether the two structures are close to each other.
 *
 * Currently it has been specialised for TVector3.
 *
 * You may specialise the template for other combound types like has been done for TVector3 below or
 * in framework/tests/Helix.cc to compare two helices.
 *
 * Note for the google test framework to properly print your type either a operator<< or PrintTo(const T& expected, std::ostream* out)
 * has to be defined.
 */
#define EXPECT_ALL_NEAR(expected, actual, tolerance) EXPECT_PRED3(::Belle2::TestHelpers::allNear<decltype(expected)>, expected, actual, tolerance)

/**
 * Assertation macro for combound structures containing floating point values like TVector3, etc. to be close to each other.
 *
 * Generally it compares each contained floating point value to be no less than the tolerance apart.
 *
 * The macro uses the templated predicate template<class T> allNear(const T& expected, const T& actual, double tolerance)
 * to compute whether the two structures are close to each other.
 *
 * Currently it has been specialised for TVector3.
 *
 * You may specialise the template for other combound types like has been done for TVector3 below or
 * in framework/tests/Helix.cc to compare two helices.
 *
 * Note for the google test framework to properly print your type either a operator<< or PrintTo(const T& expected, std::ostream* out)
 * has to be defined.
 */
#define ASSERT_ALL_NEAR(expected, actual, tolerance) ASSERT_PRED3(::Belle2::TestHelpers::allNear<decltype(expected)>, expected, actual, tolerance)

class TVector3;
namespace Belle2 {
  namespace  TestHelpers {
    /** Predicate checking that two angular values are close to each other modulus a 2 * PI difference. */
    bool angleNear(double expected, double actual, double tolerance);

    /** Predicate checking that two values have the same sign. Returns nan if any of the values is nan. */
    bool sameSign(double expected, double actual);

    /** Predicate checking that a value is bigger than zero. */
    bool isPositive(double expected);

    /** Predicate checking that a value is smaller than zero. */
    bool isNegative(double expected);

    /** Templated version of predicate checking if two combound object containing some floating point are near each other by maximum deviation.
     *  Concrete implementations can be given as simple overloads of the allNear function.
     */
    template<class T>
    bool allNear(const T& expected, const T& actual, double tolerance)
    {
      using std::fabs;
      return fabs(expected - actual) < tolerance;
    }

    /** Predicate checking that all three components of TVector3 are close by a maximal error of tolerance. */
    template<>
    bool allNear<TVector3>(const TVector3& expected, const TVector3& actual, double tolerance);

    /** Print function for the google test framework to print a TVector3 to an output stream */
    void PrintTo(const TVector3& tVector3, ::std::ostream& output);
  }
}



