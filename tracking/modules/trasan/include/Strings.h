// -*- C++ -*-
// CLASSDOC OFF
// $Id: Strings.h 10030 2007-03-09 07:51:44Z katayama $
// CLASSDOC ON
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definition of the HepString class.
//
// A simple string package for C++.  Strings have automatic
// storage management and value semantics.  Copying of the string
// contents is common in this implementation.
//
// .SS History
// Author: Dag Bruck.
// Modified slightly by Leif Lonnblad
//


#ifndef BELLE_STRINGS_H
#define BELLE_STRINGS_H

#ifdef GNUPRAGMA
#pragma interface
#endif

//iw
#ifndef BELLE_CLHEP_CLHEP_H
//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
#endif

#include <string>

namespace Belle {

  typedef std::string HepString;

#if 1
// helper functions
//
// for HepString(int)
//
  HepString itostring(int);
  HepString dtostring(double, unsigned precision = 6);
  HepString carstring(const HepString&);
  HepString cdrstring(const HepString&);
#endif


#if 0 /* Just use std::string class */

#include <stdlib.h>
#include <iostream>
#include <string.h>
#undef index

#ifdef HEP_NO_INLINE_IN_DECLARATION
#define inline
#endif

  class HepString {
  public:
    inline HepString();
    // Constructs empty string.

    HepString(char);
    // Constructs string from a character.

    HepString(const char*);
    // Constructs string from a character array.

    HepString(const HepString&);
    // Constructs copy of another string.

    HepString(int);
    // Constructs a string from an integer number.

    HepString(double, unsigned precision = 6);
    // Constructs a string from a floating point number. (Note that the precision
    // argument has no effect for the current implementation when running with
    // libg++

    inline ~HepString();
    // Destroys string, deallocating storage.

    HepString& operator = (const HepString&);
    HepString& operator = (const char*);
    HepString& operator = (char);
    // Assignment to string.

    HepString& operator += (const HepString&);
    HepString& operator += (const char*);
    HepString& operator += (char);
    // appends a string or character to string.

    HepString operator + (const HepString&);
    HepString operator + (const char*);
    HepString operator + (char);
    // Concatenates two strings, or a string with a char.

    inline char operator()(unsigned) const;
    // Returns i'th character of string.  No bounds checking!
    // Strings start at 0 (zero).

    HepString operator()(unsigned start, unsigned n) const;
    // Returns the substring starting at `start' which is at
    // most `n' characters long.  Strings start at 0 (zero).

    HepString car() const;
    // Treating the string as a white-space separated list of sub-string,
    // return the first sub-string (analogous to the car function in lisp)

    HepString cdr() const;
    // Treating the string as a white-space separated list of sub-string,
    // return the list after removing the first element (analogous to the
    // cdr function in lisp)

    inline operator const char* () const;
    // Returns pointer to contents of string.

    HepString upper() const;
    HepString lower() const;
    // Returns a string where every lower (upper) case letter is converted
    // to upper (lower) case, according to ctype(3).  Other characters are
    // copied unchanged.

    inline long toInt() const;
    inline double toFloat() const;
    // Converts string contents to integer or double.

    inline unsigned length() const;
    // Returns length of string (excluding terminating null).

    inline HepBoolean isEmpty() const;
    inline HepBoolean operator !() const;
    // Returns true if the string is empty

    inline HepBoolean operator == (const HepString&) const;
    HepBoolean operator == (const char*) const;
    inline HepBoolean operator != (const HepString&) const;
    inline HepBoolean operator != (const char*) const;
    // Tests for equality.

    inline int index(const HepString&) const;
    inline int index(char) const;
// Returns the first position of a string or character within the string.
// (The fist character has index 0.)

    inline int lIndex(char) const;
// Returns the last position of a character within the string.

  private:

    unsigned size;
    // Allocated storage for str (bytes) is equal to lengh of string + 1.

    char* str;
    // Pointer to allocated storage.

    inline void realloc();
    // Reallocates str using realloc().

    friend HepBoolean operator == (const char*, const HepString&);
    // Tests for equality ("not equal" also defined).

  };

#ifdef HEP_NO_INLINE_IN_DECLARATION
#undef inline
#endif

#ifdef HEP_SHORT_NAMES
  typedef HepString String;
#endif

  std::istream& operator >> (std::istream&, HepString&);
// Extracts a string from an input stream.  Initial whitespace
// is skipped.  The string extends until the next whitespace.

  HepString operator + (const char*, const HepString&);
// Concatenation


#ifdef HEP_DEBUG_INLINE

  std::ostream& operator << (std::ostream&, const HepString&);
  // Inserts a string in the output stream.  The same formatting
  // as for char* takes place.

  HepBoolean operator != (const char*, const HepString&);
// Test for equality

#else
//#include "belleCLHEP/String/Strings.icc"
// -*- C++ -*-
// $Id: Strings.icc 6497 2000-05-08 13:04:32Z katayama $
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definitions of the inline member functions of the
// HepString class
//

#ifdef HEP_DEBUG_INLINE
#define inline
#endif

  inline HepString::~HepString()
  {
    if (str) {
      free(str);
    }
    str = 0;
  }

  inline void HepString::realloc()
  {
    str = (char*)(str ? ::realloc(str, size * sizeof(char)) :
                  malloc(size * sizeof(char)));
  }

  inline unsigned HepString::length() const
  {
    return size - 1;
  }

  inline HepBoolean HepString::isEmpty() const
  {
    return HepBoolean(size == 1);
  }

  inline HepBoolean HepString::operator !() const
  {
    return HepBoolean(size == 1);
  }

  inline HepString::HepString()
    : size(1), str(0)
  {
    realloc();
    str[0] = '\0';
  }

  inline char HepString::operator()(unsigned i) const
  {
    return i < length() ? str[i] : '\0';
  }

  inline HepString::operator const char* () const
  {
    return str;
  }

  inline long HepString::toInt() const
  {
    return atol(str);
  }

  inline HepBoolean HepString::operator == (const HepString& s) const
  {
    return HepBoolean(str[0] == s.str[0] && strcmp(str, s.str) == 0);
  }

  inline HepBoolean HepString::operator != (const HepString& s) const
  {
    return HepBoolean(!operator == (s));
  }

  inline HepBoolean HepString::operator != (const char* s) const
  {
    return HepBoolean(!operator == (s));
  }

  inline int HepString::index(const HepString& s) const
  {
    char* i = strstr(str, s.str);
    return i ? int(i - str) : -1;
  }

  inline int HepString::index(char c) const
  {
    char* i = strchr(str, c);
    return i ? int(i - str) : -1;
  }

  inline int HepString::lIndex(char c) const
  {
    char* i = strrchr(str, c);
    return i ? int(i - str) : -1;
  }

  inline double HepString::toFloat() const
  {
    return atof(str);
  }

  inline std::ostream& operator << (std::ostream& o, const HepString& s)
  {
    return o << (const char*) s;
  }

  inline HepBoolean operator != (const char* s1, const HepString& s2)
  {
    return HepBoolean(!(s1 == s2));
  }

#ifdef HEP_DEBUG_INLINE
#undef inline
#endif

#endif
#endif /* Just use std::string class */

} // namespace Belle

#endif
