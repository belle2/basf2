#pragma once

#include <framework/logging/LogSystem.h>

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
    private:
      std::string m_oldpwd; /**< previous working directory. */
      std::string m_tmpdir; /**< path of temporary director. */
    };
  }

  /** \def EXPECT_LOGMESSAGE(x, loglevel)
   *
   *  Command x should print at least one message of given loglevel using basf2's logging system.
   *  \sa EXPECT_B2FATAL
   *  \sa EXPECT_B2ERROR
   *  \sa EXPECT_B2WARNING
   */
#define EXPECT_LOGMESSAGE(x, loglevel) \
  { \
    int nmessages_before = Belle2::LogSystem::Instance().getMessageCounter(loglevel); \
    { \
      x; \
    } \
    int nmessages_after = Belle2::LogSystem::Instance().getMessageCounter(loglevel); \
    EXPECT_TRUE(nmessages_after > nmessages_before) << "Message of level " << #loglevel << " expected, but not found."; \
  }

  /** command x should exit using B2FATAL.  */
#define EXPECT_B2FATAL(x) EXPECT_EXIT(x, ::testing::ExitedWithCode(1),"");

  /** \def EXPECT_B2ERROR(x)
   *
   *  command x should print at least one B2ERROR.
   */
#define EXPECT_B2ERROR(x) EXPECT_LOGMESSAGE(x, Belle2::LogConfig::c_Error);

  /** \def EXPECT_B2WARNING(x)
   *
   *  command x should print at least one B2WARNING.
   */
#define EXPECT_B2WARNING(x) EXPECT_LOGMESSAGE(x, Belle2::LogConfig::c_Warning);
}
