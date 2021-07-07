/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_StringUtil_hh
#define _Belle2_StringUtil_hh

#include <string>
#include <vector>

namespace Belle2 {

  typedef std::vector<std::string> StringList;

  struct StringUtil {

    static StringList split(const std::string& str,
                            const char type, size_t max = 0);
    static std::string join(StringList str_v, const std::string& s,
                            size_t start = 0, size_t end = 0);
    static std::string replace(const std::string& source,
                               const std::string& pattern,
                               const std::string& placement);
    static std::string form(const std::string& string, ...);
    static std::string toupper(const std::string& str);
    static std::string tolower(const std::string& str);
    static int atoi(const std::string str);
    static double atof(const std::string str);
    static long long atoll(const std::string str);
    static bool find(const std::string& s, const std::string& str);
    static bool isdigit(const std::string& s);

  };

};

#endif
