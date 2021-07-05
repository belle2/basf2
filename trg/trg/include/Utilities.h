/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

    /// converts int to string. (Use boost::lexical_cast)
    static std::string itostring(int i);

    /// converts double to string.
    static std::string dtostring(double d, unsigned int precision);

    /// CERNLIB car.
    static std::string carstring(const std::string& s);

    /// CERNLIB cdr.
    static std::string cdrstring(const std::string& s);

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

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGUtilities_FLAG_ */
