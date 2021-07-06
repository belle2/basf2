/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: J. Tanaka                                        *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <stdio.h>

#include <framework/logging/Logger.h>


namespace Belle2 {

  namespace analysis {

    /**
     * KFitError is a container of error codes and a macro function.
     */
    class KFitError {
    public:
      /**
       * ECode is a error code enumerate.
       */
      enum ECode {
        /** No error */
        kNoError                 =  0,

        /** Not fitted yet */
        kNotFittedYet            =  1,

        /** Specified track-id out of range */
        kOutOfRange              = 11,
        /** Unprepared */
        kUnimplemented           = 12,

        /** Track count too small to perform fit */
        kBadTrackSize            = 21,
        /** Wrong correlation matrix size */
        kBadMatrixSize           = 22,
        /** Wrong correlation matrix size (internal error) */
        kBadCorrelationSize      = 23,
        /** Bad initial chi-square (internal error) */
        kBadInitialCHIsq         = 24,

        /** Division by zero (bad track property or internal error) */
        kDivisionByZero          = 31,
        /** Cannot calculate matrix inverse (bad track property or internal error) */
        kCannotGetMatrixInverse  = 32,
        /** Cannot get arcsin (bad track property or internal error) */
        kCannotGetARCSIN         = 33,
      };


    public:
      /**
       * Display a description of error and its location.
       * @param file file name where the error takes place
       * @param line line number in the file
       * @param func function name in the file
       * @param code error code
       */
      static void displayError(const char* file, const int line, const char* func, const enum ECode code)
      {
        const char* str_error =
          code == kNoError                ? "No error" :
          code == kNotFittedYet           ? "Not fitted yet" :
          code == kOutOfRange             ? "Out of range" :
          code == kBadTrackSize           ? "Bad track count" :
          code == kBadMatrixSize          ? "Bad matrix size" :
          code == kBadCorrelationSize     ? "Bad correlation size" :
          code == kBadInitialCHIsq        ? "Bad initial chisq" :
          code == kDivisionByZero         ? "Division by zero" :
          code == kCannotGetMatrixInverse ? "Cannot calculate matrix inverse" :
          code == kCannotGetARCSIN        ? "Cannot calculate arcsin" :
          "Unknown error";

        char buf[1024];
        sprintf(buf, "%s:%d:%s(): %s", file, line, func, str_error);
        B2WARNING(buf);
      }
    };

  } // namespace analysis

} // namespace Belle2


