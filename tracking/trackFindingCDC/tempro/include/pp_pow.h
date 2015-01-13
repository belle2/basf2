/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PP_POW_H
#define PP_POW_H

#include <boost/preprocessor/arithmetic/mul.hpp>

#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <boost/preprocessor/facilities/expand.hpp>

/// Helper macro for the opening part of the repeated multiplication
#define __BELLE2_PP_POWER_PRE(z,i,base) BOOST_PP_MUL BOOST_PP_LPAREN() base BOOST_PP_COMMA()
//#define __BELLE2_PP_POWER_PRE(z,i,base) BOOST_PP_MUL ( base,

/// Helper macro for the closing part of the repeated multiplication
#define __BELLE2_PP_POWER_POST(z,i,base) BOOST_PP_RPAREN()
//#define __BELLE2_PP_POWER_POST(z,i,base) )

/// Helper macro to accumulate the opening exponent multiplications
#define __BELLE2_PP_POWER_REPEATED_PRE(exponent,base) BOOST_PP_REPEAT(exponent, __BELLE2_PP_POWER_PRE, base)

/// Helper macro to accumulate the closing exponent multiplications
#define __BELLE2_PP_POWER_REPEATED_POST(exponent) BOOST_PP_REPEAT(exponent, __BELLE2_PP_POWER_POST, ~)

/// Preprocessor version of std::pow(base,exponent) from cmath. Arguments can be non negative integers limited to BOOST_PP_LIMIT_MAG
#define BELLE2_PP_POW(base,exponent) BOOST_PP_EXPAND( __BELLE2_PP_POWER_REPEATED_PRE BOOST_PP_LPAREN () exponent BOOST_PP_COMMA() base BOOST_PP_RPAREN()   1 __BELLE2_PP_POWER_REPEATED_POST BOOST_PP_LPAREN() exponent BOOST_PP_RPAREN() )

// Note: The following commented version is the same as the former but has a pair of extra parentheses around the result, which might be not favourable if the result shall be passed to the next macro. Hence the extra preprocessor gymastics with EXPAND, LPAREN , RPAREN and COMMA
//#define BELLE2_PP_POW(base,exponent) BOOST_PP_EXPAND((  BOOST_PP_REPEAT(exponent, __BELLE2_PP_POWER_PRE, base)  1  BOOST_PP_REPEAT(exponent, __BELLE2_PP_POWER_POST, ~)  ))

#endif //PP_POW_H
