// -*- C++ -*-
// $Id: Strings.cc 10002 2007-02-26 06:56:17Z katayama $
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the implementation of the HepString class.
//
//iw
#include <sstream>
#include <iomanip>
//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
//iw #include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/Strings.h"

namespace Belle {

#if 1
//
// Helper functions for original CLHEP/String
//
  HepString itostring(int i)
  {
    std::ostringstream s;
    s << i;
    return HepString(s.str());
  }

  HepString dtostring(double d, unsigned int precision)
  {
    std::ostringstream s;
    s << std::setprecision(precision) << d;
    return HepString(s.str());
  }

  HepString carstring(const HepString& s)
  {
    HepString ret;
//    const char * p = str;
//    while ( *p && isspace(*p) ) p++;
//    while ( *p && !isspace(*p) ) ret += *(p++);
    int i;
    int len = s.length();
    for (i = 0; i < len; i++) {
      if (!isspace(s[i])) break;
    }
    for (; i < len; i++) {
      if (!isspace(s[i])) {
        ret += s[i];
      } else break;
    }
    return ret;
  }

  HepString cdrstring(const HepString& s)
  {
//    const char * p = str;
//    while ( *p && isspace(*p) ) p++;
//    while ( *p && !isspace(*p) ) p++;
//    while ( *p && isspace(*p) ) p++;
    int i;
    int len = s.length();
    for (i = 0; i < len; i++) {
      if (!isspace(s[i])) break;
    }
    for (; i < len; i++) {
      if (isspace(s[i])) break;
    }
    for (; i < len; i++) {
      if (!isspace(s[i])) break;
    }
    return HepString(s.substr(i));
  }


#endif



#if 0 /* Just use std::string class */

#ifdef GNUPRAGMA
#pragma implementation
#endif

//#include <algorithm>


#ifdef HEP_DEBUG_INLINE
#include "belleCLHEP/String/Strings.icc"
#endif

#include <ctype.h>

#ifdef WIN32
#include <strstrea.h>
#else
#include <strstream>
#endif

#if defined(__GNUG__)
// libg++ temporary workaround
#include <stdio.h>
#endif

  HepString::HepString(const HepString& s)
    : str(0)
  {
    size = s.size;
    realloc();
    strcpy(str, s.str);
  }

  HepString& HepString::operator = (const HepString& s)
  {
    if (&s != this) {
      size = s.length() + 1;
      realloc();
      strcpy(str, s.str);
    }
    return *this;
  }

  HepString HepString::car() const
  {
    HepString ret;
    const char* p = str;
    while (*p && isspace(*p)) p++;
    while (*p && !isspace(*p)) ret += *(p++);
    return ret;
  }

  HepString& HepString::operator = (char c)
  {
    if (c == '\0') {
      size = 1;
      realloc();
      str[0] = '\0';
    } else {
      size = 2;
      realloc();
      str[0] = c;
      str[1] = '\0';
    }
    return *this;
  }

  HepString HepString::operator + (const HepString& s)
  {
    HepString r = *this;
    r += s;
    return r;
  }

  HepString& HepString::operator += (const HepString& s)
  {
    size = length() + s.length() + 1;
    realloc();
    strcat(str, s.str);
    return *this;
  }

  HepString operator + (const char* s1, const HepString& s2)
  {
    HepString r(s1);
    r += s2;
    return r;
  }

  HepString HepString::operator + (char c)
  {
    HepString r = *this;
    r += c;
    return r;
  }

  HepString& HepString::operator += (char c)
  {
    if (c == '\0') return *this;
    size += 1;
    realloc();
    str[size - 2] = c;
    str[size - 1] = '\0';
    return *this;
  }

  HepString::HepString(char c)
    : str(0)
  {
    if (c == '\0') {
      size = 1;
      realloc();
      str[0] =  '\0';
    } else {
      size = 2;
      realloc();
      str[0] = c;
      str[1] = '\0';
    }
  }

  HepString HepString::cdr() const
  {
    const char* p = str;
    while (*p && isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    while (*p && isspace(*p)) p++;
    return HepString(p);
  }

  HepString::HepString(double f, unsigned precision)
    : str(0)
  {
#if defined(__GNUG__)
    // libg++ 2.7.2 has a memory leak. Temporary fix
    char tmp[128];
    std::sprintf(tmp, "%lf\0", f);
    size = strlen(tmp) + 1;
    realloc();
    strncpy(str, tmp, size - 1);
#else
    std::ostrstream os;
    os.precision(precision);
    os << f;
    size = os.pcount() + 1;
    realloc();
    strncpy(str, os.str(), size - 1);
#endif
    str[size - 1] = '\0';
  }

  HepBoolean operator == (const char* s1, const HepString& s2)
  {
    if (s1 == 0) s1 = "";
    return HepBoolean(s1[0] == s2.str[0] && strcmp(s1, s2.str) == 0);
  }

  HepString::HepString(int i)
    : str(0)
  {
#if defined(__GNUG__)
    // libg++ 2.7.2 has a memory leak. Temporary fix
    char tmp[128];
    std::sprintf(tmp, "%d\0", i);
    size = strlen(tmp) + 1;
    realloc();
    strncpy(str, tmp, size - 1);
#else
    std::ostrstream os;
    os << i;
    size = os.pcount() + 1;
    realloc();
    strncpy(str, os.str(), size - 1);
#endif
    str[size - 1] = '\0';
  }

  std::istream& operator >> (std::istream& i, HepString& s)
  {
    char c;
    s = "";

    while (i.get(c) && isspace(c));
// skip to next word

    if (i) {
      do   s += c;  while (i.get(c) && !isspace(c));
// read a word
      if (i) i.putback(c);
    }
    return i;
  }

  HepString HepString::lower() const
  {
    HepString t = *this;
    register char* p = &t.str[t.length()];
    while (--p >= t.str)
      if (isupper(*p)) *p = tolower(*p);
    return t;
  }

  HepString& HepString::operator = (const char* s)
  {
    if (s == 0) s = "";
    size = strlen(s) + 1;
    realloc();
    strcpy(str, s);
    return *this;
  }

  HepString HepString::operator + (const char* s)
  {
    HepString r = *this;
    r += s;
    return r;
  }

  HepString& HepString::operator += (const char* s)
  {
    size = length() + strlen(s) + 1;
    realloc();
    strcat(str, s);
    return *this;
  }

  HepString::HepString(const char* s)
    : str(0)
  {
    if (s == 0) s = "";
    size = strlen(s) + 1;
    realloc();
    strcpy(str, s);
  }

  HepBoolean HepString::operator == (const char* s) const
  {
    if (s == 0) s = "";
    return HepBoolean(str[0] == s[0] && strcmp(str, s) == 0);
  }

  HepString HepString::operator()(unsigned start, unsigned n) const
  {
    HepString r;
    //  int nn = std::min(int(size - start - 1), int(n));
    int nn = std::min((int)(size - start - 1), (int)(n));
    if (nn > 0) {
      r.size = nn + 1;
      r.realloc();
      strncpy(r.str, str + start, nn);
      r.str[nn] = '\0';
    }
    return r;
  }

  HepString HepString::upper() const
  {
    HepString t = *this;
    register char* p = &t.str[t.length()];
    while (--p >= t.str)
      if (islower(*p)) *p = toupper(*p);
    return t;
  }

#endif /* Just use std::string class */

} // namespace Belle
