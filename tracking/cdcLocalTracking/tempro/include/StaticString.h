/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef STATICSTRING_H
#define STATICSTRING_H

#include <stddef.h>

#include "pp_pow.h"

#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/repetition/enum.hpp>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class for compile time string handeling
    template<char... Chars>
    class StaticString;

    /// Class for compile time string handeling - specialisation taking away the first character for recursive algorithms operating on the string.
    template<char Char, char... Chars>
    class StaticString <Char, Chars...> {
    public:
      enum { size = 2 + sizeof ...(Chars) };
      static const char chars[2 + sizeof ...(Chars)];
      static const char first;
      static const char* c_str();
    };

    template<char Char, char... Chars>
    const char StaticString<Char, Chars...>::chars[2 + sizeof ...(Chars)] = {Char, Chars... , '\0' };

    template<char Char, char... Chars>
    const char StaticString<Char, Chars...>::first = Char;

    template<char Char, char... Chars>
    const char* StaticString<Char, Chars...>::c_str() { return &(chars[0]); }

    /// Class for compile time string handeling - specialisation taking no characters as end point of recursion steps.
    template<>
    class StaticString<> {
    public:
      enum { size = 1 };
      static const char chars[1];
      static const char first;
      static const char* c_str();
    };


    template<class P>
    class StaticStringTail;

    template<char Char, char... Chars>
    class StaticStringTail<StaticString<Char, Chars...> > {
    public:
      typedef StaticString<Chars...> type;
    };


    template<class P>
    class StaticStringHead;

    template<char Char, char... Chars>
    class StaticStringHead<StaticString<Char, Chars...> > {
    public:
      enum { value = Char};
    };


    template<>
    class StaticStringTail<StaticString<> > {
    public:
      enum { value = '\0' };
    };

    /// Type alias for the string with no characters.
    typedef StaticString<> EmptyStaticString;


    template<char... Chars>
    class StaticStripFront {
    public:
      typedef StaticString<Chars ...> type;
    };

    template<char... Chars>
    class StaticStripFront < '\0', Chars... > {
    public:
      typedef typename StaticStripFront<Chars ...>::type type;
    };


    template< typename S, char... Chars>
    class StaticStripBackImpl;

    /*
    template<char... ConsumedChars, char Char1,char Char2, char... Chars>
    class StaticStripBackImpl< StaticString<ConsumedChars...>, Char1, Char2, Chars...> {
    public:
      typedef typename StaticStripBackImpl<StaticString<ConsumedChars..., Char1, Char2>, Chars...>::type type;
      };*/

    /*
    template<char... ConsumedChars, BOOST_PP_ENUM_PARAMS(2, char Char), char... Chars>
    class StaticStripBackImpl< StaticString<ConsumedChars...>, BOOST_PP_ENUM_PARAMS(2, Char), Chars...> {
    public:
      typedef typename StaticStripBackImpl<StaticString<ConsumedChars..., BOOST_PP_ENUM_PARAMS(2, Char)>, Chars...>::type type;
    };*/

    template<char... ConsumedChars, char Char, char... Chars>
    class StaticStripBackImpl< StaticString<ConsumedChars...>, Char, Chars...> {
    public:
      typedef typename StaticStripBackImpl<StaticString<ConsumedChars..., Char>, Chars...>::type type;
    };

    /*
    template<char... ConsumedChars, char... Chars>
    class StaticStripBackImpl< StaticString<ConsumedChars...>, BOOST_PP_ENUM_PARAMS(2, '\0' BOOST_PP_INTERCEPT), Chars...> {
    public:
      typedef StaticString<ConsumedChars...> type;
      };*/


    template<char... ConsumedChars, char... Chars>
    class StaticStripBackImpl < StaticString<ConsumedChars...>, '\0', Chars... > {
    public:
      typedef StaticString<ConsumedChars...> type;
    };

    template<char... Chars>
    class StaticStripBack {
    public:
      typedef typename StaticStripBackImpl<StaticString<>, Chars... >::type type;
    };


  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // STATICSTRING_H
