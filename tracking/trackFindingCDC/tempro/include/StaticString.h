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
#include <type_traits>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class for compile time string handeling
    template<char... Chars>
    class StaticString;

    /// Class for compile time string handeling - specialisation taking away the first character for recursive algorithms operating on the string.
    template<char Char, char... Chars>
    class StaticString <Char, Chars...> {
    public:
      /// Class variable stating the length of the string including the trailing null character
      enum { size = 2 + sizeof ...(Chars) };

      /// Class variable of all the characters in the string including a trailing null character
      static const char chars[2 + sizeof ...(Chars)];

      /// Class variable containing the first character of the string
      static const char first;

      /// Static function returning the run time c string represented by this static string
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
      /// Class variable stating the length of the string including the trailing null character - amounts to 1 in this specialisation
      enum { size = 1 };

      /// Class variable of all the characters in the string including a trailing null character - amounts to {'\0'} in this specialisation
      static const char chars[1];

      /// Class variable containing the first character of the string - amounts to '\0' for this specialisation
      static const char first;

      /// Static function returning the run time c string represented by this static string - amounts to "\0" for this specialisation
      static const char* c_str();

    };

    /// Type alias for the string with no characters.
    typedef StaticString<> EmptyStaticString;



    /// Implementation of the metafunction, which retieves a StaticString without the first character.
    template<class P>
    struct StaticStringTailImpl;

    /// Implementation of the metafunction which retieves a StaticString without the first character - specialisation for non null strings.
    template<char Char, char... Chars>
    struct StaticStringTailImpl<StaticString<Char, Chars...> > {
      /// Result of the metafunction containing the StaticString without the first character.
      typedef StaticString<Chars...> type;
    };

    /// Implementation of the metafunction which retieves a StaticString without the first character - specialisation null strings.
    template<>
    struct StaticStringTailImpl< StaticString<> > {
      /// Result of the metafunction containing the StaticString without the first character.
      typedef StaticString<> type;
    };



    /// Metafunction returning the StaticString without the first character.
    template<class StaticString_>
    using StaticStringTail = typename StaticStringTailImpl<StaticString_ >::type;

    /// Implementation of the metafunction, which retieves the first character of a StaticString. Specialisations always inherit from std::integral_constant, which contains the result.
    template<class P>
    struct StaticStringHead;

    /// Implementation of the metafunction, which retieves the first character of a StaticString - specialisation for non empty strings.
    template<char Char, char... Chars>
    struct StaticStringHead<StaticString<Char, Chars...> > :
        std::integral_constant<char, Char> {
    };

    /// Implementation of the metafunction, which retieves the first character of a StaticString - specialisation for empty strings.
    template<>
    struct StaticStringHead<StaticString<> > :
        std::integral_constant < char, '\0' > {
    };

    /* As soon c++14 and templated variables are available this can be activated for direct usage of the metafunction with the need of ::value as in the current StaticStringHeadImpl version */
    /*
    template<char... Chars>
    constexpr char staticStringHead = StaticStringHead<StaticString<Chars...>::value;
    */

    /// Implementation of the metafunction stripping leading null characters from a variadic character sequence.
    template<char... Chars>
    struct StaticStripFrontImpl {
      /// Result of the metafunction - a StaticString made from the variadic character sequence stripped from leading null characters
      typedef StaticString<Chars ...> type;
    };

    /// Implementation of the metafunction stripping leading null characters from a variadic character sequence - specialisation that strips one leading null character.
    template<char... Chars>
    struct StaticStripFrontImpl < '\0', Chars... > {
      /// Result of the metafunction - a StaticString made from the variadic character sequence stripped from leading null characters.
      typedef typename StaticStripFrontImpl<Chars ...>::type type;
    };

    /// Metafunction striping null characters from the start StaticString
    template<char ... Chars>
    using StaticStripFront = typename StaticStripFrontImpl<Chars...>::type;



    /// Implementation of the metafunction, which strips trailing characters from a variadic character sequence after the first null character.
    template< typename S, char... Chars>
    struct StaticStripBackImpl;

    /// Implementation of the metafunction which strips of superflicious null characters from a StaticString - specialisation to capture the a null character and aborting the recursion
    template<char... ConsumedChars, char... Chars>
    struct StaticStripBackImpl < StaticString<ConsumedChars...>, '\0', Chars... > {
      /// Result of the metafunction - a StaticString containing only the characters before the first null character.
      typedef StaticString<ConsumedChars...> type;
    };

    /// Implementation of the metafunction which strips of superflicious null characters from a StaticString - specialisation to capture a non null character and recursively move it to the resulting StaticString.
    template<char... ConsumedChars, char Char, char... Chars>
    struct StaticStripBackImpl< StaticString<ConsumedChars...>, Char, Chars...> {
      /// Result of the metafunction - a StaticString containing only the characters before the first null character.
      typedef typename StaticStripBackImpl<StaticString<ConsumedChars..., Char>, Chars...>::type type;
    };

    /// Metafunction returning a StaticString with characters before the first null character.
    template<char... Chars>
    using StaticStripBack =  typename StaticStripBackImpl<StaticString<>, Chars... >::type;


  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // STATICSTRING_H
