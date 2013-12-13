//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TDebug.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Debug utility functions
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TDebug_FLAG_
#define TDebug_FLAG_

#include <vector>
#include <string>

namespace Belle {

/// A class for debugging of TSIM
class TDebug {

  public:

    /// returns the debug level.
    static int level(void);

    /// sets and returns the debug level.
    static int level(int newLevel, bool thisStageOnly = false);

    /// Declare that you enter new stage.
    static void enterStage(const std::string & stageName);

    /// Declare that you leave a stage. 
    static void leaveStage(const std::string & stageName);

    /// returns tab spaces.
    static std::string tab(void);

    /// returns tab spaces with extra spaces
    static std::string tab(int extra);

  private:

    /// Storage for stages.
    static std::vector<std::string> _stages;

    /// Debug level. 0:do nothing, 1:show you flow, 2:show you detail,
    /// 10:show you very detail
    static int _level;
};

//-----------------------------------------------------------------------------

//...Trasan stage information...
std::string Stage(void);
std::string Tab(int shift = 0);
unsigned EnterStage(const std::string&);
unsigned LeaveStage(const std::string&);
unsigned NestLevel(void);
void DumpStageInformation(void);

//...Structure for stage analyses...
struct stageInfo {
    unsigned counter;
    double usedUTime;
    double lastUTime;
    double usedSTime;
    double lastSTime;
};

/// A class to provide debug utility functions
class TDebugUtilities {

  public: // Utility functions

    /// converts int to string. (Use boost::lexical_cast)
    static std::string itostring(int i);

    /// converts double to string.
    static std::string dtostring(double d, unsigned int precision);

    /// CERNLIB car.
    static std::string carstring(const std::string &s);

    /// CERNLIB cdr.
    static std::string cdrstring(const std::string &s);

    /// returns date string.
    static std::string dateString(void);

    /// returns date string for filename.
    static std::string dateStringF(void);

    /// Dumps bit contents to cout.
    static void bitDisplay(unsigned);

    /// Dumps bit contents to cout.
    static void bitDisplay(unsigned val,
                           unsigned firstDigit,
                           unsigned lastDigit);

    /// Dumps bit stream in string.
    static std::string streamDisplay(unsigned);

    /// Dumps bit stream in string.
    static std::string streamDisplay(unsigned,
				     unsigned firstDigit,
				     unsigned lastDigit);

};



} // namespace Belle

#endif /* TDebug_FLAG_ */
