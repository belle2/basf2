//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Utilities.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Utility functions
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGUtilities_FLAG_
#define TRGUtilities_FLAG_

#include <string>

#ifdef TRG_SHORT_NAMES
#define TRGUtil TRGUtilities
#endif

namespace Belle2 {

/// A class to provide TRG utility functions
class TRGUtilities {

  public: // Utility functions

    /// converts int to string.
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
};

//-----------------------------------------------------------------------------

#ifdef TRG_NO_INLINE
#define inline
#else
#undef inline
#define TRGUtilities_INLINE_DEFINE_HERE
#endif

#ifdef TRGUtilities_INLINE_DEFINE_HERE



#endif

#undef inline

} // namespace Belle2

#endif /* TRGUtilities_FLAG_ */
