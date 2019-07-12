/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#define BOOST_SPIRIT_UNICODE
#include <boost/spirit/include/qi.hpp>
#include <analysis/DecayDescriptor/DecayStringParticle.h>
#include <analysis/DecayDescriptor/DecayStringDecay.h>
#include <analysis/DecayDescriptor/DecayString.h>
#include <vector>
#include <string>

namespace Belle2 {
  /** This class describes the grammar and the syntax elements of decay strings.
  It is used to parse a given decay string to C++ structs which are then used
  to initialise the DecayDescriptor class.

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.*/
  template <typename Iterator>
  struct DecayStringGrammar : boost::spirit::qi::grammar<Iterator, DecayString(), boost::spirit::unicode::space_type> {
    DecayStringGrammar() : DecayStringGrammar::base_type(start)
  {
    using boost::spirit::unicode::char_;
    using boost::spirit::unicode::string;
    using boost::spirit::unicode::space;
    using boost::spirit::qi::lit;
    using boost::spirit::qi::lexeme;
    using boost::spirit::repeat;

    // Reserved characters for steering - cppcheck doesn't understand the
    // boost::spirit syntax so we suppress warnings
    // cppcheck-suppress knownConditionTrueFalse
    reserved = space || '^' || '[' || ']' || '>' || ':' || '.' || '?' || '!' || '@';

    // particle composed of selector, particle name, and user label: "^D_s+:label"
    particle %= *selector >> lexeme[+(char_ - reserved)] >> -label;
    selector = string("^") | string("@");
    label %= lit(":") >> lexeme[+(char_ - reserved)];

    // Arrow types
    arrow %= string("->") | string("-->") | string("=>") | string("==>");

    // Keyword for custom MC Matching
    reserved_keyword = string("...") | string("?nu") | string("!nu") | string("?rad") | string("!rad");
    keywordlist = *reserved_keyword;

    // Basic decay: mother -> daughterlist
    decay %= particle >> arrow >> daughterlist >> -keywordlist;
    daughterdecay %= lit("[") >> decay >> lit("]");
    daughter %= daughterdecay | particle;
    daughterlist %= +daughter;

    // This rule is where the parser starts
    // The decay string can be either a single decay or a list of decays
    start %=  decay | particle;
  }
  /** Reserved characters that are not allowed in particle names or labels. */
  boost::spirit::qi::rule<Iterator> reserved;
  /** Particle in the decay string: 'selector name label'. */
  boost::spirit::qi::rule<Iterator, DecayStringParticle(), boost::spirit::unicode::space_type> particle;
  /** Particles can be selected by preceeding '^' symbol. */
  boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::unicode::space_type> selector;
  /** Label that can be attached to a particle name, separated by the '/' symbol. */
  boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::unicode::space_type> label;
  /** Allowed arrow types. */
  boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::unicode::space_type> arrow;
  /** Syntax keyword */
  boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::unicode::space_type> reserved_keyword;
  boost::spirit::qi::rule<Iterator, std::vector<std::string>(), boost::spirit::unicode::space_type> keywordlist;
  /** Syntax of a decay: 'mother arrow daughters ...'. */
  boost::spirit::qi::rule<Iterator, DecayStringDecay(), boost::spirit::unicode::space_type> decay;
  /** Syntax of decaying daughter particle. Daughter decays have to be in brackets '[ ]'.*/
  boost::spirit::qi::rule<Iterator, DecayStringDecay(), boost::spirit::unicode::space_type> daughterdecay;
  /** A daughter particle which can be either a plain particle or a decaying particle. */
  boost::spirit::qi::rule<Iterator, DecayString(), boost::spirit::unicode::space_type> daughter;
  /** The list of the daughters, i.e. what is on the right side of the arrow. */
  boost::spirit::qi::rule<Iterator, std::vector<DecayString>(), boost::spirit::unicode::space_type> daughterlist;
  /** The rule where the parser starts. This corresponds to the full decay string. */
  boost::spirit::qi::rule<Iterator, DecayString(), boost::spirit::unicode::space_type> start;
  };
}
